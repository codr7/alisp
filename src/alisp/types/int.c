#include <stdio.h>
#include "alisp/type.h"
#include "alisp/types/int.h"
#include "alisp/val.h"

static void copy_val(struct a_val *dst, struct a_val *src) { dst->as_int = src->as_int; }

static void dump_val(struct a_val *val) { printf("%d", val->as_int); }

static bool true_val(struct a_val *val) { return val->as_int; }

struct a_type *a_int_type_init(struct a_type *self, struct a_vm *vm, struct a_string *name, struct a_type *super_types[]) {
  a_type_init(self, vm, name, super_types);
  self->copy_val = copy_val;
  self->dump_val = dump_val;
  self->true_val = true_val;
  return self;
}
