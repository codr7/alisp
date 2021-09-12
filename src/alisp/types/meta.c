#include "alisp/type.h"
#include "alisp/fail.h"
#include "alisp/string.h"
#include "alisp/val.h"

static void clone_val(struct a_val *dst, struct a_val *src) { dst->as_meta = a_type_clone(src->as_meta, dst->type->vm); }

static void copy_val(struct a_val *dst, struct a_val *src) { dst->as_meta = src->as_meta; }

static void dump_val(struct a_val *val) { printf("%s", val->as_meta->name->data); }

static bool is_val(struct a_val *x, struct a_val *y) { return x->as_meta == y->as_meta; }

struct a_type *a_meta_type_init(struct a_type *self,
				struct a_vm *vm,
				struct a_string *name,
				struct a_type *super[]) {
  a_type_init(self, vm, name, super);
  self->clone_val = clone_val;
  self->copy_val = copy_val;
  self->dump_val = dump_val;
  self->is_val = is_val;
  return self;
}
