#include "alisp/type.h"
#include "alisp/fail.h"
#include "alisp/func.h"
#include "alisp/stack.h"
#include "alisp/string.h"
#include "alisp/val.h"
#include "alisp/vm.h"

static a_pc_t call_val(struct a_val *val, enum a_call_flags flags, a_pc_t ret) {
  struct a_vm *vm = val->type->vm;
  struct a_func *f = val->as_func;
  if ((flags & A_CALL_CHECK) && !a_func_applicable(f, vm)) {
    a_stack_dump(&vm->stack);
    a_fail("Func not applicable: %s", f->name->data);
    return NULL;
  }
  return a_func_call(f, vm, flags, ret);
}

static void copy_val(struct a_val *dst, struct a_val *src) { dst->as_func = src->as_func; }

static void dump_val(struct a_val *val) { printf("Func(%s)", val->as_func->name->data); }

struct a_type *a_func_type_init(struct a_type *self,
				struct a_vm *vm,
				struct a_string *name,
				struct a_type *super[]) {
  a_type_init(self, vm, name, super);
  self->call_val = call_val;
  self->copy_val = copy_val;
  self->dump_val = dump_val;
  return self;
}
