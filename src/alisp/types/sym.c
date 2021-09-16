#include <stdio.h>
#include "alisp/string.h"
#include "alisp/type.h"
#include "alisp/val.h"
#include "alisp/vm.h"

static enum a_order compare_val(struct a_val *x, struct a_val *y) { return a_string_compare(x->as_sym, y->as_sym); }

static void clone_val(struct a_val *dst, struct a_val *src) {
  dst->as_sym = a_string(dst->type->vm, src->as_sym->data);
}

static void copy_val(struct a_val *dst, struct a_val *src) { dst->as_sym = src->as_sym; }

static void dump_val(struct a_val *val) { printf("'%s", val->as_sym->data); }

static bool is_val(struct a_val *x, struct a_val *y) { return x->as_sym == y->as_sym; }

struct a_type *a_sym_type_init(struct a_type *self, struct a_vm *vm, struct a_string *name, struct a_type *super[]) {
  a_type_init(self, vm, name, super);
  self->compare_val = compare_val;
  self->clone_val = clone_val;
  self->copy_val = copy_val;
  self->dump_val = dump_val;
  self->is_val = is_val;
  return self;
}
