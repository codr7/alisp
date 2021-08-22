#include <assert.h>
#include <stddef.h>
#include "alisp/type.h"

struct a_type *a_type_init(struct a_type *self, struct a_vm *vm) {
  self->vm = vm;
  self->copy_val = NULL;
  self->deref_val = NULL;
  self->refs = 1;
  return self;
}

struct a_type *a_type_ref(struct a_type *self) {
  self->refs++;
  return self;
}

bool a_type_deref(struct a_type *self) {
  assert(self->refs);
  if (--self->refs) { return false; }
  return true;
}
