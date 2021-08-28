#include <assert.h>
#include "alisp/func.h"
#include "alisp/vm.h"

struct a_func *a_func_init(struct a_func *self,
			   struct a_string *name,
			   struct a_args *args,
			   struct a_rets *rets) {
  self->name = name;
  self->args = args;
  self->rets = rets;
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
  return true;
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

a_pc a_func_call(struct a_func *self, struct a_vm *vm, a_pc ret) {
  assert(self->body);
  return self->body(self, vm, ret);
}
