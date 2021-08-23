#include "alisp/type.h"
#include "alisp/fail.h"
#include "alisp/string.h"
#include "alisp/types/meta.h"
#include "alisp/val.h"

static void copy_val(struct a_val *dst, struct a_val *src) {
  dst->as_meta = a_type_ref(src->as_meta);
}

static bool deref_val(struct a_val *val) {
  return a_type_deref(val->as_meta);
}

struct a_type *a_meta_type_init(struct a_type *self, struct a_vm *vm, struct a_string *name) {
  a_type_init(self, vm, name);
  self->copy_val = copy_val;
  self->deref_val = deref_val;
  return self;
}
