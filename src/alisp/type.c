#include <assert.h>
#include <stddef.h>
#include "alisp/string.h"
#include "alisp/type.h"

struct a_type *a_type_init(struct a_type *self, struct a_vm *vm, struct a_string *name) {
  self->vm = vm;
  self->name = a_string_ref(name);

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
  a_string_deref(self->name);
  return true;
}
