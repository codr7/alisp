#include <assert.h>
#include <stdio.h>
#include "alisp/fail.h"
#include "alisp/frame.h"
#include "alisp/func.h"
#include "alisp/stack.h"
#include "alisp/string.h"
#include "alisp/vm.h"

struct a_mem {
  struct a_ls ls;
  struct a_ls args;
  struct a_ls rets;
};

static const void *mem_key(const struct a_ls *self) {
  return &a_baseof(self, struct a_mem, ls)->args;
}

static enum a_order mem_compare(const void *x, const void *y) {
  const struct a_ls *xls = x, *yls = y;
  
  for (;;) {
    xls = xls->next;
    yls = yls->next;
    
    if (xls == x && yls != y) { return A_LT; }
    if (xls != x && yls == y) { return A_GT; }

    switch (a_compare(a_baseof(xls, struct a_val, ls), a_baseof(yls, struct a_val, ls))) {
    case A_LT:
      return A_LT;
    case A_GT:
      return A_GT;
    case A_EQ:
      break;
    }
  }

  return A_EQ;
}

struct a_func *a_func(struct a_vm *vm,
		      struct a_string *name,
		      struct a_args *args,
		      struct a_rets *rets) {
  return a_func_init(a_malloc(vm, sizeof(struct a_func)), name, args, rets);
}

struct a_func *a_func_init(struct a_func *self,
			   struct a_string *name,
			   struct a_args *args,
			   struct a_rets *rets) {
  self->name = name;
  self->args = args;
  self->rets = rets;
  a_lset_init(&self->mem, mem_key, mem_compare);
  self->start_pc = NULL;
  self->scope = NULL;
  self->reg_count = 0;
  self->body = NULL;
  self->ref_count = 1;
  return self;
}

struct a_func *a_func_ref(struct a_func *self) {
  self->ref_count++;
  return self;
}

bool a_func_deref(struct a_func *self, struct a_vm *vm) {
  assert(self->ref_count);
  if (--self->ref_count) { return false; }
  a_free(vm, self->args);
  a_free(vm, self->rets);
  if (self->scope) { a_scope_deref(self->scope); }
  return true;
}

static a_reg_t push_reg(struct a_func *self, a_reg_t reg) {
  self->regs[self->reg_count++] = reg;
  return reg;
}

void a_func_begin(struct a_func *self, struct a_vm *vm) {
  a_emit(vm, A_GOTO_OP);
  self->start_pc = a_pc(vm);
  self->scope = a_begin(vm);

  for (struct a_arg *a = self->args->items; a < self->args->items+self->args->count; a++) {
    if (a->name) { a->reg = push_reg(self, a_scope_bind_reg(self->scope, a->name)); }
  }
}

static struct a_arg *find_arg(struct a_func *self, a_reg_t reg) {
  for (struct a_arg *a = self->args->items; a < self->args->items + self->args->count; a++) {
    if (a->reg == reg) { return a; }
  }

  return NULL;
}

void a_func_end(struct a_func *self, struct a_vm *vm) {
  a_emit(vm, A_RET_OP)->as_ret.func = self;
  a_end(vm);
  a_baseof(self->start_pc, struct a_op, pc)->as_goto.pc = a_pc(vm);

  for (a_pc_t pc = self->start_pc->next; ; pc = pc->next) {
    struct a_op *op = a_baseof(pc, struct a_op, pc);
    if (op->type == A_RET_OP && op->as_ret.func == self) { break; }
    
    if (op->type == A_LOAD_OP) {
      struct a_arg *a = find_arg(self, op->as_load.reg);
      if (a) { op->as_load.type = a->type; }
    }
  }
}

bool a_func_applicable(struct a_func *self, struct a_vm *vm) {
  struct a_ls *s = vm->stack.prev;
  
  for (struct a_arg *a = self->args->items + self->args->count-1; a >= self->args->items; a--, s = s->prev) {
    if (s == &vm->stack) { return false; }
    if (!a_isa(a_baseof(s, struct a_val, ls)->type, a->type)) { return false; }
  }
  
  return true;
}

a_pc_t a_func_call(struct a_func *self, struct a_vm *vm, enum a_call_flags flags, a_pc_t ret) {
  if (self->body) {
    ret = self->body(self, vm, ret);
    if (flags & A_CALL_DRETS) { a_drop(vm, self->rets->count); }
    return ret;
  }

  if (!(flags & A_CALL_TCO)) { a_push_frame(vm, self, flags, ret); }
  struct a_ls *sp = vm->stack.prev;
  
  for (struct a_arg *a = self->args->items+self->args->count-1; a >= self->args->items; a--) {
    assert(sp != &vm->stack);

    if (a->name) {
      struct a_val *rv = a_scope_find(self->scope, a->name);

      if (rv == NULL) {
	a_fail("Missing argument binding: %s", a->name->data);
	return NULL;
      }
      
      assert(rv->type == &vm->abc.reg_type);
      struct a_ls *vls = sp;
      sp = sp->prev;
      struct a_val *v = a_baseof(a_ls_pop(vls), struct a_val, ls);

      if (v == NULL) {
	a_fail("Missing argument: %s", a->name->data);
	return NULL;
      }

      a_store(vm, rv->as_reg, v);
    } else {
      sp = sp->prev;
    }
  }
  
  return self->start_pc;
}

struct a_args *a_args(struct a_vm *vm, uint8_t count) {
  struct a_args *args =	a_malloc(vm, sizeof(struct a_args) + count*sizeof(struct a_arg));	
  args->count = count;
  return args;
}

struct a_rets *a_rets(struct a_vm *vm, uint8_t count) {
  struct a_rets *rets =	a_malloc(vm, sizeof(struct a_rets) + count*sizeof(struct a_type *));	
  rets->count = count;
  return rets;
}
