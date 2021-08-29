#include <assert.h>
#include <stdio.h>
#include "alisp/fail.h"
#include "alisp/frame.h"
#include "alisp/func.h"
#include "alisp/stack.h"
#include "alisp/string.h"
#include "alisp/vm.h"

struct a_func *a_func(struct a_vm *vm,
		      struct a_string *name,
		      struct a_args *args,
		      struct a_rets *rets) {
  return a_func_init(a_malloc(&vm->func_pool, sizeof(struct a_func)), name, args, rets);
}

struct a_func *a_func_init(struct a_func *self,
			   struct a_string *name,
			   struct a_args *args,
			   struct a_rets *rets) {
  self->name = name;
  self->args = args;
  self->rets = rets;
  self->start_pc = NULL;
  self->scope = NULL;
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
  a_free(&vm->pool, self->args);
  a_free(&vm->pool, self->rets);
  if (self->scope) { a_scope_deref(self->scope); }
  return true;
}

void a_func_begin(struct a_func *self, struct a_vm *vm) {
  a_emit(vm, A_GOTO_OP);
  self->start_pc = a_pc(vm);
  self->scope = a_begin(vm);

  for (struct a_arg *a = self->args->items; a < self->args->items+self->args->count; a++) {
    if (a->name) { a_scope_bind_reg(self->scope, a->name); }
  }
}

void a_func_end(struct a_func *self, struct a_vm *vm) {
  a_emit(vm, A_RET_OP);
  a_end(vm);
  a_baseof(self->start_pc, struct a_op, ls)->as_goto.pc = a_pc(vm);
}

bool a_func_applicable(struct a_func *self, struct a_vm *vm) {
  if (!self->args) { return true; }
  struct a_ls *s = vm->stack.prev;
  
  for (struct a_arg *a = self->args->items + self->args->count-1; a >= self->args->items; a--) {
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

  struct a_frame *f = a_frame_init(a_malloc(&vm->frame_pool, sizeof(struct a_frame)), vm, self, ret);
  a_ls_push(&vm->frames, &f->ls);
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
      struct a_val *v = a_baseof(a_ls_pop(sp), struct a_val, ls);

      if (v == NULL) {
	a_fail("Missing argument: %s", a->name->data);
	return NULL;
      }

      a_store(vm, rv->as_reg, v);
    }

    sp = sp->prev;
  }
  
  return self->start_pc;
}

struct a_args *a_args(struct a_vm *vm, uint8_t count) {
  struct a_args *args =	a_malloc(&vm->pool, sizeof(struct a_args) + count*sizeof(struct a_arg));	
  args->count = count;
  return args;
}

struct a_rets *a_rets(struct a_vm *vm, uint8_t count) {
  struct a_rets *rets =	a_malloc(&vm->pool, sizeof(struct a_rets) + count*sizeof(struct a_type *));	
  rets->count = count;
  return rets;
}
