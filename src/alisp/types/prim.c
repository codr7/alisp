#include "alisp/type.h"
#include "alisp/fail.h"
#include "alisp/prim.h"
#include "alisp/string.h"
#include "alisp/val.h"

static void copy_val(struct a_val *dst, struct a_val *src) {
  dst->as_prim = src->as_prim;
}

static void dump_val(struct a_val *val) { printf("Prim(%s)", val->as_prim->name->data); }

struct a_type *a_prim_type_init(struct a_type *self,
				struct a_vm *vm,
				struct a_string *name,
				struct a_type *super[]) {
  a_type_init(self, vm, name, super);
  self->copy_val = copy_val;
  self->dump_val = dump_val;
  return self;
}
