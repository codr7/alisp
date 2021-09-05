#include <stdio.h>
#include "alisp/fail.h"
#include "alisp/func.h"
#include "alisp/form.h"
#include "alisp/libs/math.h"
#include "alisp/stack.h"
#include "alisp/string.h"
#include "alisp/vm.h"

static a_pc_t fix_body(struct a_func *self, struct a_vm *vm, a_pc_t ret) {
  struct a_val *scale = a_pop(vm), *val = a_peek(vm, 0);
  a_val_init(val, &vm->math.fix_type)->as_fix = a_fix(val->as_int, scale->as_int);
  a_val_deref(scale);
  a_val_free(scale, vm);
  return ret;
}

static a_pc_t frac_body(struct a_func *self, struct a_vm *vm, a_pc_t ret) {
  struct a_val *val = a_peek(vm, 0);
  val->as_fix = a_fix(a_fix_frac(val->as_fix), val->as_fix.scale);
  return ret;
}

static a_pc_t scale_of_body(struct a_func *self, struct a_vm *vm, a_pc_t ret) {
  struct a_val *val = a_peek(vm, 0);
  a_val_init(val, &vm->abc.int_type)->as_int = val->as_fix.scale;
  return ret;
}

static a_pc_t trunc_body(struct a_func *self, struct a_vm *vm, a_pc_t ret) {
  struct a_val *val = a_peek(vm, 0);
  a_val_init(val, &vm->abc.int_type)->as_int = a_fix_trunc(val->as_fix);
  return ret;
}

static a_pc_t add_fix_body(struct a_func *self, struct a_vm *vm, a_pc_t ret) {
  struct a_val *y = a_pop(vm), *x = a_peek(vm, 0);
  x->as_fix = a_fix_add(x->as_fix, y->as_fix);
  a_val_deref(y);
  a_val_free(y, vm);
  return ret;
}

static a_pc_t sub_fix_body(struct a_func *self, struct a_vm *vm, a_pc_t ret) {
  struct a_val *y = a_pop(vm), *x = a_peek(vm, 0);
  x->as_fix = a_fix_sub(x->as_fix, y->as_fix);
  a_val_deref(y);
  a_val_free(y, vm);
  return ret;
}

static a_pc_t add_int_body(struct a_func *self, struct a_vm *vm, a_pc_t ret) {
  struct a_val *y = a_pop(vm), *x = a_peek(vm, 0);
  x->as_int += y->as_int;
  a_val_deref(y);
  a_val_free(y, vm);
  return ret;
}

static a_pc_t sub_int_body(struct a_func *self, struct a_vm *vm, a_pc_t ret) {
  struct a_val *y = a_pop(vm), *x = a_peek(vm, 0);
  x->as_int -= y->as_int;
  a_val_deref(y);
  a_val_free(y, vm);
  return ret;
}

struct a_math_lib *a_math_lib_init(struct a_math_lib *self, struct a_vm *vm) {
  a_lib_init(&self->lib, vm, a_string(vm, "math"));

  a_lib_bind_type(&self->lib, a_fix_type_init(&self->fix_type, vm, a_string(vm, "Fix"),
					      A_SUPER(&vm->abc.any_type, &vm->abc.num_type)));

  a_lib_bind_func(&self->lib,
		  a_func(vm, a_string(vm, "fix"),
			 A_ARG(vm,
			       {a_string(vm, "val"), &vm->abc.int_type},
			       {a_string(vm, "scale"), &vm->abc.int_type}),
			 A_RET(vm, &self->fix_type)))->body = fix_body;

  a_lib_bind_func(&self->lib,
		  a_func(vm, a_string(vm, "frac"),
			 A_ARG(vm, {a_string(vm, "val"), &self->fix_type}),
			 A_RET(vm, &self->fix_type)))->body = frac_body;

  a_lib_bind_func(&self->lib,
		  a_func(vm, a_string(vm, "scale-of"),
			 A_ARG(vm, {a_string(vm, "val"), &self->fix_type}),
			 A_RET(vm, &vm->abc.int_type)))->body = scale_of_body;

  a_lib_bind_func(&self->lib,
		  a_func(vm, a_string(vm, "trunc"),
			 A_ARG(vm, {a_string(vm, "val"), &self->fix_type}),
			 A_RET(vm, &vm->abc.int_type)))->body = trunc_body;

  a_lib_bind_func(&self->lib,
		  a_func(vm, a_string(vm, "+"),
			 A_ARG(vm,
			       {a_string(vm, "x"), &self->fix_type},
			       {a_string(vm, "y"), &self->fix_type}),
			 A_RET(vm, &self->fix_type)))->body = add_fix_body;

  a_lib_bind_func(&self->lib,
		  a_func(vm, a_string(vm, "-"),
			 A_ARG(vm,
			       {a_string(vm, "x"), &self->fix_type},
			       {a_string(vm, "y"), &self->fix_type}),
			 A_RET(vm, &self->fix_type)))->body = sub_fix_body;

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
