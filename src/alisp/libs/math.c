#include <stdio.h>
#include "alisp/fail.h"
#include "alisp/func.h"
#include "alisp/form.h"
#include "alisp/libs/math.h"
#include "alisp/stack.h"
#include "alisp/string.h"
#include "alisp/vm.h"

static a_pc_t add_int_body(struct a_func *self, struct a_vm *vm, a_pc_t ret) {
  struct a_val *y = a_pop(vm), *x = a_peek(vm, 0);
  x->as_int += y->as_int;
  a_val_deref(y);
  a_free(vm, y);
  return ret;
}

static a_pc_t sub_int_body(struct a_func *self, struct a_vm *vm, a_pc_t ret) {
  struct a_val *y = a_pop(vm), *x = a_peek(vm, 0);
  x->as_int -= y->as_int;
  a_val_deref(y);
  a_free(vm, y);
  return ret;
}

struct a_math_lib *a_math_lib_init(struct a_math_lib *self, struct a_vm *vm) {
  a_lib_init(&self->lib, vm, a_string(vm, "math"));
  
  a_lib_bind_func(&self->lib,
		  a_func(vm, a_string(vm, "+"),
			 A_ARG(vm,
			       {a_string(vm, "x"), &vm->abc.int_type},
			       {a_string(vm, "y"), &vm->abc.int_type}),
			 A_RET(vm, &vm->abc.int_type)))->body = add_int_body;

  a_lib_bind_func(&self->lib,
		  a_func(vm, a_string(vm, "-"),
			 A_ARG(vm,
			       {a_string(vm, "x"), &vm->abc.int_type},
			       {a_string(vm, "y"), &vm->abc.int_type}),
			 A_RET(vm, &vm->abc.int_type)))->body = sub_int_body;

  return self;
}
