#include "alisp/type.h"
#include "alisp/fail.h"
#include "alisp/func.h"
#include "alisp/string.h"
#include "alisp/val.h"

static a_pc call_val(struct a_val *val, enum a_call_flags flags, a_pc ret) {
  struct a_func *f = val->as_func;
  if ((flags & A_CALL_CHECK) && !a_func_applicable(f, val->type->vm)) { a_fail("Func not applicable: %s", f->name->data); }
  return a_func_call(f, val->type->vm, flags, ret);
}

static void copy_val(struct a_val *dst, struct a_val *src) {
  dst->as_func = a_func_ref(src->as_func);
}

static bool deref_val(struct a_val *val) { return a_func_deref(val->as_func, val->type->vm); }

static void dump_val(struct a_val *val) { printf("Func(%p)", val->as_func); }


struct a_type *a_func_type_init(struct a_type *self,
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
