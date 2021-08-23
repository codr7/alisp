#include "alisp/libs/abc.h"
#include "alisp/string.h"
#include "alisp/types/int.h"
#include "alisp/types/reg.h"

struct a_abc_lib *a_abc_lib_init(struct a_abc_lib *self, struct a_vm *vm) {
  a_int_type_init(&self->int_type, vm, a_string(vm, "Int"));
  a_reg_type_init(&self->reg_type, vm, a_string(vm, "Reg"));
  return self;
}
