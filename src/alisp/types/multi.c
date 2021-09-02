#include "alisp/type.h"
#include "alisp/fail.h"
#include "alisp/multi.h"
#include "alisp/string.h"
#include "alisp/val.h"

static a_pc_t call_val(struct a_val *val, enum a_call_flags flags, a_pc_t ret) {
  return a_multi_call(val->as_multi, val->type->vm, flags, ret);
}

static void copy_val(struct a_val *dst, struct a_val *src) {
  dst->as_multi = a_multi_ref(src->as_multi);
}

static bool deref_val(struct a_val *val) { return a_multi_deref(val->as_multi, val->type->vm); }

static void dump_val(struct a_val *val) { printf("Multi(%s)", val->as_multi->name->data); }

struct a_type *a_multi_type_init(struct a_type *self,
				struct a_vm *vm,
				struct a_string *name,
				struct a_type *super[]) {
  a_type_init(self, vm, name, super);
  self->call_val = call_val;
  self->copy_val = copy_val;
  self->deref_val = deref_val;
  self->dump_val = dump_val;
  return self;
}
