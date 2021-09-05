#include <stdio.h>
#include "alisp/type.h"
#include "alisp/val.h"

static enum a_order compare_val(struct a_val *x, struct a_val *y) { return a_fix_compare(x->as_fix, y->as_fix); }

static void copy_val(struct a_val *dst, struct a_val *src) { dst->as_fix = src->as_fix; }

static void dump_val(struct a_val *val) { a_fix_dump(val->as_fix); }

static bool is_val(struct a_val *x, struct a_val *y) {
  struct a_fix *xf = &x->as_fix, *yf = &y->as_fix;
  return xf->scale == yf->scale && xf->val == yf->val;
}

static bool true_val(struct a_val *val) { return val->as_fix.val; }

struct a_type *a_fix_type_init(struct a_type *self, struct a_vm *vm, struct a_string *name, struct a_type *super[]) {
  a_type_init(self, vm, name, super);
  self->compare_val = compare_val;
  self->copy_val = copy_val;
  self->dump_val = dump_val;
  self->is_val = is_val;
  self->true_val = true_val;
  return self;
}
