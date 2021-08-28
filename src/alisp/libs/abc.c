#include <stdio.h>
#include "alisp/fail.h"
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

static bool alias_body(struct a_prim *self, struct a_vm *vm, struct a_ls *args, uint8_t arg_count) {
  struct a_ls *a = args->next;
  struct a_form *org = a_baseof(a, struct a_form, ls);

  if (org->type != A_ID_FORM) {
    a_fail("Invalid id form: %d", org->type);
    return false;
  }

  struct a_scope *s = a_scope(vm);
  struct a_val *v = a_scope_find(s, org->as_id.name);

  if (!v) {
    a_fail("Unknown id: %s", org->as_id.name->data);
    return false;
  }
  
  struct a_form *new = a_baseof(a->next, struct a_form, ls);

  if (new->type != A_ID_FORM) {
    a_fail("Invalid id form: %d", new->type);
    return false;
  }

  a_copy(a_scope_bind(s, new->as_id.name, v->type), v);
  return true;
}

static bool d_body(struct a_prim *self, struct a_vm *vm, struct a_ls *args, uint8_t arg_count) {
  int count = 1;
  
  if (!a_ls_null(args)) {
    struct a_form *a = a_baseof(args->next, struct a_form, ls);
    struct a_val *v = a_form_val(a, vm);

    if (v) {
      if (v->type != &vm->abc.int_type) {
	a_fail("Invalid drop count: %s", v->type->name->data);
	return false;
      }

      count = v->as_int;
    } else {
      a_form_emit(a, vm);
      count = -1;
    }
  }
  
  a_emit(vm, A_DROP_OP)->as_drop.count = count;
  return true;
}

static bool do_body(struct a_prim *self, struct a_vm *vm, struct a_ls *args, uint8_t arg_count) {
  a_ls_do(args, a) {
    if (!a_form_emit(a_baseof(a, struct a_form, ls), vm)) { return false; }
  }

  return true;
}

static bool if_body(struct a_prim *self, struct a_vm *vm, struct a_ls *args, uint8_t arg_count) {
  struct a_ls *a = args->next;
  struct a_form *cond = a_baseof(a, struct a_form, ls);
  if (!a_form_emit(cond, vm)) { return false; }

  struct a_branch_op *branch = &a_emit(vm, A_BRANCH_OP)->as_branch;
  struct a_form *left = a_baseof((a = a->next), struct a_form, ls), *right = NULL;
  if (!a_form_emit(left, vm)) { return false; }

  if ((a = a->next) != args) {
    struct a_goto_op *skip_right = &a_emit(vm, A_GOTO_OP)->as_goto;
    branch->right_pc = a_next_pc(vm);
    right = a_baseof(a, struct a_form, ls);
    if (!a_form_emit(right, vm)) { return false; }
    skip_right->pc = a_next_pc(vm);
  } else {
    branch->right_pc = a_next_pc(vm);
  }

  return true;
}

static bool reset_body(struct a_prim *self, struct a_vm *vm, struct a_ls *args, uint8_t arg_count) {
  a_emit(vm, A_RESET_OP);
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
  a_lib_bind_type(&self->lib, a_prim_type_init(&self->prim_type, vm, a_string(vm, "Prim"), (struct a_type *[]){NULL}));
  
  a_lib_bind(&self->lib, a_string(vm, "ALISP-VERSION"), &self->int_type)->as_int = A_VERSION;
  a_lib_bind(&self->lib, a_string(vm, "T"), &self->bool_type)->as_bool = true;
  a_lib_bind(&self->lib, a_string(vm, "F"), &self->bool_type)->as_bool = false;

  a_lib_bind_prim(&self->lib, a_prim(vm, a_string(vm, "alias"), 2, 2))->body = alias_body;
  a_lib_bind_prim(&self->lib, a_prim(vm, a_string(vm, "d"), 0, 1))->body = d_body;
  a_lib_bind_prim(&self->lib, a_prim(vm, a_string(vm, "do"), 0, -1))->body = do_body;
  a_lib_bind_prim(&self->lib, a_prim(vm, a_string(vm, "if"), 2, 3))->body = if_body;
  a_lib_bind_prim(&self->lib, a_prim(vm, a_string(vm, "reset"), 0, 0))->body = reset_body;
  return self;
}
