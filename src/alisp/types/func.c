#include "alisp/type.h"
#include "alisp/fail.h"
#include "alisp/func.h"
#include "alisp/string.h"
#include "alisp/types/func.h"
#include "alisp/val.h"

static a_pc call_val(struct a_val *val, a_pc ret, bool check) {
  struct a_func *f = val->as_func;
  if (check && !a_func_applicable(f)) { a_fail("Func not applicable: %s", f->name->data); }
  return a_func_call(f, ret);
}

static void copy_val(struct a_val *dst, struct a_val *src) {
  dst->as_func = a_func_ref(src->as_func);
}

static void dump_val(struct a_val *val) { printf("Func(%p)", val->as_func); }

static bool deref_val(struct a_val *val) {
  return a_func_deref(val->as_func);
}

struct a_type *a_func_type_init(struct a_type *self,
				struct a_vm *vm,
				struct a_string *name,
				struct a_type *super_types[]) {
  a_type_init(self, vm, name, super_types);
  self->call_val = call_val;
  self->copy_val = copy_val;
  self->dump_val = dump_val;
  self->deref_val = deref_val;
  return self;
}
