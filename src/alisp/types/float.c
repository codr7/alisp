#include <stdio.h>
#include "alisp/type.h"
#include "alisp/val.h"

static enum a_order compare_val(struct a_val *x, struct a_val *y) {
  if (x->as_float < y->as_float) { return A_LT; }
  return (x->as_float == y->as_float) ? A_EQ : A_GT;
}

static void copy_val(struct a_val *dst, struct a_val *src) { dst->as_float = src->as_float; }

static void dump_val(struct a_val *val) { printf("%Lf", val->as_float); }

static bool is_val(struct a_val *x, struct a_val *y) { return x->as_float == y->as_float; }

static bool true_val(struct a_val *val) { return val->as_float; }

struct a_type *a_float_type_init(struct a_type *self, struct a_vm *vm, struct a_string *name, struct a_type *super[]) {
  a_type_init(self, vm, name, super);
  self->compare_val = compare_val;
  self->copy_val = copy_val;
  self->dump_val = dump_val;
  self->is_val = is_val;
  self->true_val = true_val;
  return self;
}
