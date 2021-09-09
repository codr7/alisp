#include <stdio.h>
#include "alisp/iter.h"
#include "alisp/string.h"
#include "alisp/type.h"
#include "alisp/val.h"

static void copy_val(struct a_val *dst, struct a_val *src) { dst->as_iter = a_iter_ref(src->as_iter); }

static bool deref_val(struct a_val *val) { return a_iter_deref(val->as_iter, val->type->vm); }

static void dump_val(struct a_val *val) { printf("Iter(%p)", val->as_iter); }

static struct a_iter *iter_val(struct a_val *val) { return val->as_iter; }

struct a_type *a_iter_type_init(struct a_type *self,
				struct a_vm *vm,
				struct a_string *name,
				struct a_type *super[]) {
  a_type_init(self, vm, name, super);
  self->copy_val = copy_val;
  self->deref_val = deref_val;
  self->dump_val = dump_val;
  self->iter_val = iter_val;
  return self;
}
