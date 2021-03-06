#include <assert.h>
#include <stdio.h>
#include "alisp/fail.h"
#include "alisp/frame.h"
#include "alisp/func.h"
#include "alisp/stack.h"
#include "alisp/string.h"
#include "alisp/vm.h"

static const void *mem_key(const struct a_ls *self) {
  return &a_baseof(self, struct a_func_mem, ls)->args;
}

static enum a_order mem_compare(const void *x, const void *y) {
  const struct a_ls *xls = x, *yls = y;
  
  for (;;) {
    xls = xls->next;
    yls = yls->next;
    if (xls == x || yls == y) { break; }

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

struct a_func *a_func_new(struct a_vm *vm,
			  struct a_string *name,
			  struct a_args args,
			  struct a_rets rets) {
  return a_func_init(a_pool_alloc(&vm->func_pool), name, args, rets);
}

struct a_func *a_func_init(struct a_func *self,
			   struct a_string *name,
			   struct a_args args,
			   struct a_rets rets) {
  self->name = name;
  self->args = args;
  self->rets = rets;
  a_lset_init(&self->mem, mem_key, mem_compare);
  self->start_pc = NULL;
  self->scope = NULL;
  self->min_reg = self->max_reg = -1;
  self->body = NULL;
  self->weight = 0;
  
  for (struct a_arg *a = self->args.items; a < self->args.items+self->args.count; a++) {
    self->weight += a->type->id;
  }
  
  return self;
}

struct a_func *a_func_clone(struct a_func *self, struct a_vm *dst_vm, struct a_vm *src_vm) {
  a_check(self->body, "Clone not supported");
  struct a_string *dn = a_string(dst_vm, self->name->data);
  struct a_val *found = a_scope_find(&dst_vm->main, dn);

  if (found) {
    assert(found->type == &dst_vm->abc.func_type);
    return found->as_func;
  }

  struct a_func *dst = a_func_new(dst_vm, dn, A_ARG(dst_vm), A_RET(dst_vm));

  for (struct a_arg *s = self->args.items, *d = dst->args.items; s < self->args.items + self->args.count; s++, d++) {
    d->name = a_string(dst_vm, s->name->data);
    d->type = a_type_clone(s->type, dst_vm);
    d->reg = s->reg;
    dst->args.count++;
  }

  for (struct a_type **s = self->rets.items, **d = dst->rets.items; s < self->rets.items + self->rets.count; s++, d++) {
    *d = a_type_clone(*s, dst_vm);
    dst->rets.count++;
  }

  dst->body = self->body;
  a_scope_bind(&dst_vm->main, dst->name, &dst_vm->abc.func_type)->as_func = dst;
  return dst;
}

static a_reg_t push_reg(struct a_func *self, a_reg_t reg) { return (self->max_reg = reg); }

void a_func_begin(struct a_func *self, struct a_vm *vm) {
  a_emit(vm, A_GOTO_OP);
  self->start_pc = a_pc(vm);
  self->scope = a_begin(vm);
  self->min_reg = self->scope->next_reg;
  
  for (struct a_arg *a = self->args.items; a < self->args.items+self->args.count; a++) {
    if (a->name) { a->reg = push_reg(self, a_scope_bind_reg(self->scope, a->name)); }
  }
}

static struct a_arg *find_arg(struct a_func *self, a_reg_t reg) {
  for (struct a_arg *a = self->args.items; a < self->args.items + self->args.count; a++) {
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
  
  for (struct a_arg *a = self->args.items + self->args.count-1; a >= self->args.items; a--, s = s->prev) {
    if (!a_isa(a_baseof(s, struct a_val, ls)->type, a->type)) { return false; }
  }
  
  return true;
}

a_pc_t a_func_call(struct a_func *self, struct a_vm *vm, enum a_call_flags flags, a_pc_t ret) {
  struct a_func_mem *mem = NULL;
  
  if (flags & A_CALL_MEM) {
    assert(self->args.count);
    struct a_ls *sp = &vm->stack;
    for (int i = 0; i < self->args.count; i++, sp = sp->prev);
    struct a_ls *found = a_lset_find(&self->mem, sp->prev);
    
    if (found) {
      a_drop(vm, 0, self->args.count);
      
      a_ls_do(&a_baseof(found, struct a_func_mem, ls)->rets, ls) {
	struct a_val *v = a_baseof(ls, struct a_val, ls);
	a_copy(a_push(vm, v->type), v);
      }
      
      return ret;
    } else {
      mem = a_malloc(vm, sizeof(struct a_func_mem));
      struct a_ls *sp = vm->stack.prev;
      a_ls_init(&mem->args);
      
      for (int i = 0; i < self->args.count && sp != &vm->stack; i++, sp = sp->prev) {
	struct a_val *src = a_baseof(sp, struct a_val, ls), *dst = a_val_new(src->type);
	a_copy(dst, src);
	a_ls_push(mem->args.next, &dst->ls);
      }
    }
  }
  
  if (self->body) {
    ret = self->body(self, vm, ret);
    if (mem) { a_func_mem(self, vm, mem); }
    if (flags & A_CALL_DRETS) { a_drop(vm, 0, self->rets.count); }
    return ret;
  }

  if (mem || !(flags & A_CALL_TCO)) { a_push_frame(vm, self, flags, mem, ret); }
  struct a_ls *sp = vm->stack.prev;
  
  for (struct a_arg *a = self->args.items+self->args.count-1; a >= self->args.items; a--) {
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

void a_func_mem(struct a_func *self, struct a_vm *vm, struct a_func_mem *mem) {  
  struct a_ls *sp = vm->stack.prev;
  a_ls_init(&mem->rets);

  for (int i = 0; i < self->rets.count; i++, sp = sp->prev) {
    struct a_val *src = a_baseof(sp, struct a_val, ls), *dst = a_val_new(src->type);
    a_copy(dst, src);
    a_ls_push(mem->rets.next, &dst->ls);
  }

  a_lset_add(&self->mem, &mem->ls, false);
}
