#include <assert.h>
#include "alisp/func.h"
#include "alisp/string.h"
#include "alisp/vm.h"

struct a_func *a_func_init(struct a_func *self,
			   struct a_vm *vm,
			   struct a_string *name,
			   int arg_count, struct a_arg args[],
			   struct a_type *rets[]) {
  self->vm = vm;
  self->name = a_string_ref(name);
  self->arg_count = arg_count;
  
  if (arg_count) {
    self->args = a_malloc(&vm->pool, sizeof(struct a_arg) * arg_count);
    for (int i = 0; i < arg_count; i++) { self->args[i] = args[i]; }
  } else {
    self->args = NULL;
  }

  self->ret_count = 0;
  for (struct a_type **t = rets; *t; t++, self->ret_count++);

  if (self->ret_count) {
    self->rets = a_malloc(&vm->pool, sizeof(struct a_type *) * self->ret_count);
    for (int i = 0; i < self->ret_count; i++) { self->rets[i] = rets[i]; }
  } else {
    self->rets = NULL;
  }

  self->body = NULL;
  self->ref_count = 1;
  return self;
}

struct a_func *a_func_ref(struct a_func *self) {
  self->ref_count++;
  return self;
}

bool a_func_deref(struct a_func *self) {
  assert(self->ref_count);
  if (--self->ref_count) { return false; }
  a_string_deref(self->name);

  if (self->args) {
    //TODO deinit args
    a_free(&self->vm->pool, self->args);
  }
  
  return true;
}

bool a_func_applicable(struct a_func *self) {
  struct a_ls *s = self->vm->stack.prev;
  if (!self->args) { return true; }
  
  for (int i = self->arg_count-1; i >= 0; i--, s = s->prev) {
    if (s == &self->vm->stack) { return false; }
    struct a_val *v = a_baseof(s, struct a_val, ls);
    if (!a_isa(v->type, self->args[i].type)) { return false; }
  }
  
  return true;
}

a_pc a_func_call(struct a_func *self, a_pc ret) {
  assert(self->body);
  return self->body(self, ret);
}

struct a_arg a_arg(struct a_string *name, struct a_type *type) {
  return (struct a_arg){.name=a_string_ref(name), .type=type};
}
