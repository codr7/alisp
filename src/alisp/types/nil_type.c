#include <stdio.h>
#include "alisp/type.h"
#include "alisp/val.h"

static void copy_val(struct a_val *dst, struct a_val *src) { }

static void dump_val(struct a_val *val) { printf("NIL"); }

static bool is_val(struct a_val *x, struct a_val *y) { return true; }

static bool true_val(struct a_val *val) { return false; }

struct a_type *a_nil_type_init(struct a_type *self, struct a_vm *vm, struct a_string *name, struct a_type *super[]) {
  a_type_init(self, vm, name, super);
  self->copy_val = copy_val;
  self->dump_val = dump_val;
  self->is_val = is_val;
  self->true_val = true_val;
  return self;
}
