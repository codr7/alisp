#include <stdio.h>
#include "alisp/string.h"
#include "alisp/type.h"
#include "alisp/val.h"

static enum a_order compare_val(struct a_val *x, struct a_val *y) {
  return a_string_compare(x->as_string, y->as_string);
}

static void copy_val(struct a_val *dst, struct a_val *src) { dst->as_string = src->as_string; }

static void dump_val(struct a_val *val) { printf("\"%s\"", val->as_string->data); }

static bool is_val(struct a_val *x, struct a_val *y) { return x->as_string == y->as_string; }

static bool true_val(struct a_val *val) { return val->as_string->length; }

struct a_type *a_string_type_init(struct a_type *self, struct a_vm *vm, struct a_string *name, struct a_type *super[]) {
  a_type_init(self, vm, name, super);
  self->compare_val = compare_val;
  self->copy_val = copy_val;
  self->dump_val = dump_val;
  self->is_val = is_val;
  self->true_val = true_val;
  return self;
}
