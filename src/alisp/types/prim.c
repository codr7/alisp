#include "alisp/type.h"
#include "alisp/fail.h"
#include "alisp/prim.h"
#include "alisp/string.h"
#include "alisp/val.h"

static void copy_val(struct a_val *dst, struct a_val *src) {
  dst->as_prim = a_prim_ref(src->as_prim);
}

static void dump_val(struct a_val *val) { printf("Prim(%p)", val->as_prim); }

static bool deref_val(struct a_val *val) {
  return a_prim_deref(val->as_prim);
}

struct a_type *a_prim_type_init(struct a_type *self,
				struct a_vm *vm,
				struct a_string *name,
				struct a_type *super[]) {
  a_type_init(self, vm, name, super);
  self->copy_val = copy_val;
  self->dump_val = dump_val;
  self->deref_val = deref_val;
  return self;
}
