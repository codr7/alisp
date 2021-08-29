#include "alisp/stack.h"
#include "alisp/type.h"
#include "alisp/val.h"

static enum a_order compare_val(struct a_val *x, struct a_val *y) {
  switch (a_compare(x->as_pair.left, y->as_pair.left)) {
  case A_LT:
    return A_LT;
  case A_GT:
    return A_GT;
  case A_EQ:
    break;
  }
  
  switch (a_compare(x->as_pair.right, y->as_pair.right)) {
  case A_LT:
    return A_LT;
  case A_GT:
    return A_GT;
  case A_EQ:
    break;
  }

  return A_EQ;
}

static void copy_val(struct a_val *dst, struct a_val *src) { dst->as_pair = src->as_pair; }

static bool deref_val(struct a_val *val) {
  return a_val_deref(val->as_pair.left) && a_val_deref(val->as_pair.right);
}

static void dump_val(struct a_val *val) { a_pair_dump(&val->as_pair); }

static bool equals_val(struct a_val *x, struct a_val *y) {
  return a_equals(x->as_pair.left, y->as_pair.left) && a_equals(x->as_pair.right, y->as_pair.right);
}

static bool is_val(struct a_val *x, struct a_val *y) {
  return a_is(x->as_pair.left, y->as_pair.left) && a_is(x->as_pair.right, y->as_pair.right);
}

static bool true_val(struct a_val *val) {
  return a_true(val->as_pair.left) && a_true(val->as_pair.right);
}

struct a_type *a_pair_type_init(struct a_type *self, struct a_vm *vm, struct a_string *name, struct a_type *super[]) {
  a_type_init(self, vm, name, super);
  self->compare_val = compare_val;
  self->copy_val = copy_val;
  self->deref_val = deref_val;
  self->dump_val = dump_val;
  self->equals_val = equals_val;
  self->is_val = is_val;
  self->true_val = true_val;
  return self;
}
