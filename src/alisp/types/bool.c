#include <stdio.h>
#include "alisp/type.h"
#include "alisp/val.h"

static enum a_order compare_val(struct a_val *x, struct a_val *y) {
  if (!x->as_bool && y->as_bool) { return A_LT; }
  return (x->as_bool == y->as_bool) ? A_EQ : A_GT;
}

static void copy_val(struct a_val *dst, struct a_val *src) { dst->as_bool = src->as_bool; }

static void dump_val(struct a_val *val) { putc(val->as_bool ? 'T' : 'F', stdout); }

static bool is_val(struct a_val *x, struct a_val *y) { return x->as_bool == y->as_bool; }

static bool true_val(struct a_val *val) { return val->as_bool; }

struct a_type *a_bool_type_init(struct a_type *self,
				struct a_vm *vm,
				struct a_string *name,
				struct a_type *super[]) {
  a_type_init(self, vm, name, super);
  self->compare_val = compare_val;
  self->copy_val = copy_val;
  self->dump_val = dump_val;
  self->is_val = is_val;
  self->true_val = true_val;
  return self;
}
