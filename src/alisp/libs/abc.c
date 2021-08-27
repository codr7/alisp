#include "alisp/form.h"
#include "alisp/libs/abc.h"
#include "alisp/prim.h"
#include "alisp/string.h"
#include "alisp/types/bool.h"
#include "alisp/types/func.h"
#include "alisp/types/int.h"
#include "alisp/types/ls.h"
#include "alisp/types/meta.h"
#include "alisp/types/prim.h"
#include "alisp/types/reg.h"
#include "alisp/vm.h"

static bool if_body(struct a_prim *self, struct a_vm *vm, struct a_ls *args, uint8_t arg_count) {
  struct a_ls *a = args->next;
  struct a_form *cond = a_baseof(a, struct a_form, ls);
  if (!a_form_emit(cond, vm)) { return false; }

  struct a_branch_op *branch = &a_emit(vm, A_BRANCH_OP)->as_branch;
  struct a_form *left = a_baseof((a = a->next), struct a_form, ls), *right = NULL;
  if (!a_form_emit(left, vm)) { return false; }
  branch->right_pc = a_next_pc(vm);

  if ((a = a->next) != args) {
    struct a_goto_op *skip_right = &a_emit(vm, A_GOTO_OP)->as_goto;
    right = a_baseof(a, struct a_form, ls);
    if (!a_form_emit(right, vm)) { return false; }
    skip_right->pc = a_next_pc(vm);
  }

  return true;
}

struct a_abc_lib *a_abc_lib_init(struct a_abc_lib *self, struct a_vm *vm) {
  a_lib_init(&self->lib, vm, a_string(vm, "abc"));
  
  a_lib_bind_type(&self->lib, a_bool_type_init(&self->bool_type, vm, a_string(vm, "Bool"), (struct a_type *[]){NULL}));
  a_lib_bind_type(&self->lib, a_int_type_init(&self->int_type, vm, a_string(vm, "Int"), (struct a_type *[]){NULL}));
  a_lib_bind_type(&self->lib, a_ls_type_init(&self->ls_type, vm, a_string(vm, "Ls"), (struct a_type *[]){NULL}));
  a_lib_bind_type(&self->lib, a_meta_type_init(&self->meta_type, vm, a_string(vm, "Meta"), (struct a_type *[]){NULL}));
  a_lib_bind_type(&self->lib, a_func_type_init(&self->func_type, vm, a_string(vm, "Func"), (struct a_type *[]){NULL}));
  a_lib_bind_type(&self->lib, a_reg_type_init(&self->reg_type, vm, a_string(vm, "Reg"), (struct a_type *[]){NULL}));
  a_lib_bind_type(&self->lib, a_reg_type_init(&self->prim_type, vm, a_string(vm, "Prim"), (struct a_type *[]){NULL}));
  
  a_lib_bind(&self->lib, a_string(vm, "ALISP-VERSION"), &self->int_type)->as_int = A_VERSION;
  a_lib_bind(&self->lib, a_string(vm, "T"), &self->bool_type)->as_bool = true;
  a_lib_bind(&self->lib, a_string(vm, "F"), &self->bool_type)->as_bool = false;

  a_lib_bind_prim(&self->lib, a_prim(vm, a_string(vm, "if"), 2, 3))->body = if_body;
  return self;
}
