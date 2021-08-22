#include "alisp/type.h"
#include "alisp/types/int.h"
#include "alisp/val.h"

static void copy_val(struct a_val *dst, struct a_val *src) {
  dst->as_int = src->as_int;
}

struct a_type *a_int_type_init(struct a_type *self, struct a_vm *vm, struct a_string *name) {
  a_type_init(self, vm, name);
  self->copy_val = copy_val;
  return self;
}
