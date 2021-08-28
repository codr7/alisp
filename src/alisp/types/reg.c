#include <inttypes.h>
#include <stdio.h>
#include "alisp/type.h"
#include "alisp/types/reg.h"
#include "alisp/val.h"

static void copy_val(struct a_val *dst, struct a_val *src) {
  dst->as_reg = src->as_reg;
}

static void dump_val(struct a_val *val) { printf("Prim(%" PRIu16 ")", val->as_reg); }

struct a_type *a_reg_type_init(struct a_type *self, struct a_vm *vm, struct a_string *name, struct a_type *super_types[]) {
  a_type_init(self, vm, name, super_types);
  self->copy_val = copy_val;
  self->dump_val = dump_val;
  return self;
}