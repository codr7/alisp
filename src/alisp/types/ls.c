#include "alisp/stack.h"
#include "alisp/type.h"
#include "alisp/types/ls.h"
#include "alisp/val.h"

static void copy_val(struct a_val *dst, struct a_val *src) { dst->as_ls = src->as_ls; }

static void dump_val(struct a_val *val) { a_stack_dump(val->as_ls); }

static bool true_val(struct a_val *val) { return !a_ls_null(val->as_ls); }

struct a_type *a_ls_type_init(struct a_type *self, struct a_vm *vm, struct a_string *name, struct a_type *super_types[]) {
  a_type_init(self, vm, name, super_types);
  self->copy_val = copy_val;
  self->dump_val = dump_val;
  self->true_val = true_val;
  return self;
}
