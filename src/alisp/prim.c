#include <assert.h>
#include "alisp/prim.h"
#include "alisp/vm.h"

struct a_prim *a_prim_init(struct a_prim *self,
			   struct a_string *name,
			   uint8_t min_args, uint8_t max_args) {
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

bool a_prim_call(struct a_prim *self, struct a_vm *vm, struct a_ls *args) {
  assert(self->body);
  return self->body(self, args);
}
