#include "alisp/stack.h"
#include "alisp/type.h"
#include "alisp/val.h"

static enum a_order compare_val(struct a_val *x, struct a_val *y) {
  for (struct a_ls *xls = x->as_ls->next, *yls = y->as_ls->next; true; xls = xls->next, yls = yls->next) {
    if (xls == x->as_ls && yls != y->as_ls) { return A_LT; }
    if (xls != x->as_ls && yls == y->as_ls) { return A_GT; }
    if (xls == x->as_ls && yls == y->as_ls) { break; }

    switch (a_compare(a_baseof(xls, struct a_val, ls), a_baseof(yls, struct a_val, ls))) {
    case A_LT:
      return A_LT;
    case A_GT:
      return A_GT;
    case A_EQ:
      break;
    }
  }

  return A_EQ;
}

static void copy_val(struct a_val *dst, struct a_val *src) { dst->as_ls = src->as_ls; }

static void dump_val(struct a_val *val) { a_stack_dump(val->as_ls); }

static bool equals_val(struct a_val *x, struct a_val *y) {
  for (struct a_ls *xls = x->as_ls->next, *yls = y->as_ls->next; true; xls = xls->next, yls = yls->next) {
    if (xls == x->as_ls && yls != y->as_ls) { return false; }
    if (xls != x->as_ls && yls == y->as_ls) { return false; }
    if (xls == x->as_ls && yls == y->as_ls) { break; }
    if (!a_equals(a_baseof(xls, struct a_val, ls), a_baseof(yls, struct a_val, ls))) { return false; }
  }

  return true;
}

static bool is_val(struct a_val *x, struct a_val *y) { return x->as_ls == y->as_ls; }

static bool true_val(struct a_val *val) { return !a_ls_null(val->as_ls); }

struct a_type *a_ls_type_init(struct a_type *self, struct a_vm *vm, struct a_string *name, struct a_type *super[]) {
  a_type_init(self, vm, name, super);
  self->compare_val = compare_val;
  self->copy_val = copy_val;
  self->dump_val = dump_val;
  self->equals_val = equals_val;
  self->is_val = is_val;
  self->true_val = true_val;
  return self;
}
