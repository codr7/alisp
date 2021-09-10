#include <inttypes.h>
#include <stdio.h>
#include "alisp/iter.h"
#include "alisp/type.h"
#include "alisp/val.h"
#include "alisp/vm.h"

static enum a_order compare_val(struct a_val *x, struct a_val *y) {
  if (x->as_int < y->as_int) { return A_LT; }
  return (x->as_int == y->as_int) ? A_EQ : A_GT;
}

static void copy_val(struct a_val *dst, struct a_val *src) { dst->as_int = src->as_int; }

static void dump_val(struct a_val *val) { printf("%" PRId64, val->as_int); }

static bool is_val(struct a_val *x, struct a_val *y) { return x->as_int == y->as_int; }

static struct a_val *iter_body(struct a_iter *self, struct a_vm *vm) {
  struct a_val
    *i = a_baseof(self->data.next, struct a_val, ls),
    *max = a_baseof(self->data.next->next, struct a_val, ls);

  if (i->as_int == max->as_int) { return NULL; }
  struct a_val *out = a_val_new(&vm->abc.int_type);
  out->as_int = i->as_int++;
  return out;
}

static struct a_iter *iter_val(struct a_val *val) {
  struct a_vm *vm = val->type->vm;
  struct a_iter *it = a_iter_new(vm, iter_body);
  struct a_val *i = a_val_new(&vm->abc.int_type), *max = a_val_new(&vm->abc.int_type);
  i->as_int = 0;
  max->as_int = val->as_int;
  a_ls_push(&it->data, &i->ls);
  a_ls_push(&it->data, &max->ls);
  return it;
}

static bool true_val(struct a_val *val) { return val->as_int; }

struct a_type *a_int_type_init(struct a_type *self, struct a_vm *vm, struct a_string *name, struct a_type *super[]) {
  a_type_init(self, vm, name, super);
  self->compare_val = compare_val;
  self->copy_val = copy_val;
  self->dump_val = dump_val;
  self->is_val = is_val;
  self->iter_val = iter_val;
  self->true_val = true_val;
  return self;
}
