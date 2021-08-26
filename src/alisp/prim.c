#include <assert.h>
#include "alisp/fail.h"
#include "alisp/prim.h"
#include "alisp/vm.h"

struct a_prim *a_prim(struct a_vm *vm,
		      struct a_string *name,
		      int8_t min_args, int8_t max_args) {
  return a_prim_init(a_malloc(&vm->prim_pool, sizeof(struct a_prim)), name, min_args, max_args);
}


struct a_prim *a_prim_init(struct a_prim *self,
			   struct a_string *name,
			   int8_t min_args, int8_t max_args) {
  self->name = name;
  self->min_args = min_args;
  self->max_args = max_args;
  self->body = NULL;
  self->ref_count = 1;
  return self;
}

struct a_prim *a_prim_ref(struct a_prim *self) {
  self->ref_count++;
  return self;
}

bool a_prim_deref(struct a_prim *self) {
  assert(self->ref_count);
  if (--self->ref_count) { return false; }
  return true;
}

bool a_prim_call(struct a_prim *self, struct a_vm *vm, struct a_ls *args, uint8_t arg_count) {
  assert(self->body);
  
  if ((self->min_args != -1 && arg_count < self->min_args) ||
      (self->max_args != -1 && arg_count > self->max_args)) {
    a_fail("Wrong number of arguments: %" PRIu8, arg_count);
    return false;
  }
  
  return self->body(self, vm, args, arg_count);
}
