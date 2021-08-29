#include <stdio.h>
#include "alisp/fail.h"
#include "alisp/form.h"
#include "alisp/func.h"
#include "alisp/libs/abc.h"
#include "alisp/prim.h"
#include "alisp/stack.h"
#include "alisp/string.h"
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

static a_pc_t equals_body(struct a_func *self, struct a_vm *vm, a_pc_t ret) {
  struct a_val *y = a_pop(vm), *x = a_pop(vm);
  a_push(vm, &vm->abc.bool_type)->as_bool = a_equals(x, y);
  a_val_deref(x);
  a_free(&vm->val_pool, x);
  a_val_deref(y);
  a_free(&vm->val_pool, y);
  return ret;
}

static a_pc_t lt_body(struct a_func *self, struct a_vm *vm, a_pc_t ret) {
  struct a_val *y = a_pop(vm), *x = a_pop(vm);
  a_push(vm, &vm->abc.bool_type)->as_bool = a_compare(x, y) == A_LT;
  a_val_deref(x);
  a_free(&vm->val_pool, x);
  a_val_deref(y);
  a_free(&vm->val_pool, y);
  return ret;
}

static a_pc_t gt_body(struct a_func *self, struct a_vm *vm, a_pc_t ret) {
  struct a_val *y = a_pop(vm), *x = a_pop(vm);
  a_push(vm, &vm->abc.bool_type)->as_bool = a_compare(x, y) == A_GT;
  a_val_deref(x);
  a_free(&vm->val_pool, x);
  a_val_deref(y);
  a_free(&vm->val_pool, y);
  return ret;
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
    branch->right_pc = a_pc(vm);
    right = a_baseof(a, struct a_form, ls);
    if (!a_form_emit(right, vm)) { return false; }
    skip_right->pc = a_pc(vm);
  } else {
    branch->right_pc = a_pc(vm);
  }

  return true;
}

static a_pc_t is_body(struct a_func *self, struct a_vm *vm, a_pc_t ret) {
  struct a_val *y = a_pop(vm), *x = a_pop(vm);
  a_push(vm, &vm->abc.bool_type)->as_bool = a_is(x, y);
  a_val_deref(x);
  a_free(&vm->val_pool, x);
  a_val_deref(y);
  a_free(&vm->val_pool, y);
  return ret;
}

static bool let_body(struct a_prim *self, struct a_vm *vm, struct a_ls *args, uint8_t arg_count) {
  struct a_scope *s = a_scope(vm);
  struct a_form *bsf = a_baseof(args->next, struct a_form, ls);

  if (bsf->type != A_LS_FORM) {
    a_fail("Invalid bindings: %d", bsf->type);
    return false;
  }

  struct a_ls *bs = &bsf->as_ls.items, *b = bs->prev;

  while (b != bs) {
    struct a_form *vf = a_baseof(b, struct a_form, ls);

    if ((b = b->prev) == bs) {
      a_fail("Malformed bindings");
      return false;
    }

    struct a_form *kf = a_baseof(b, struct a_form, ls);
    b = b->prev;    

    if (kf->type != A_ID_FORM) {
      a_fail("Invalid key form: %d", kf->type);
      return false;
    }
    
    struct a_string *k = kf->as_id.name;
    struct a_val *v = a_form_val(vf, vm);
    
    if (v) {
      a_copy(a_scope_bind(s, k, v->type), v);
    } else {
      a_reg_t reg = a_bind_reg(vm, k);
      if (!a_form_emit(vf, vm)) { return false; }
      a_emit(vm, A_STORE_OP)->as_store.reg = reg;
    }
  }

  for (struct a_ls *fls = args->next->next; fls != args; fls = fls->next) {
    if (!a_form_emit(a_baseof(fls, struct a_form, ls), vm)) { return false; }
  }

  bs = &bsf->as_ls.items;
  b = bs->next;

  for (b = bs->next; b != bs; b = b->next->next) {
    struct a_form *kf = a_baseof(b, struct a_form, ls);
    a_scope_unbind(s, kf->as_id.name);
  }
  
  return true;
}

static bool reset_body(struct a_prim *self, struct a_vm *vm, struct a_ls *args, uint8_t arg_count) {
  a_emit(vm, A_RESET_OP);
  return true;
}

struct a_abc_lib *a_abc_lib_init(struct a_abc_lib *self, struct a_vm *vm) {
  a_lib_init(&self->lib, vm, a_string(vm, "abc"));
  a_lib_bind_type(&self->lib, a_type_init(&self->any_type, vm, a_string(vm, "Any"), (struct a_type *[]){NULL}));
  
  a_lib_bind_type(&self->lib, a_bool_type_init(&self->bool_type, vm, a_string(vm, "Bool"),
					       A_SUPER(&self->any_type)));
  
  a_lib_bind_type(&self->lib, a_int_type_init(&self->int_type, vm, a_string(vm, "Int"),
					      A_SUPER(&self->any_type)));
  
  a_lib_bind_type(&self->lib, a_ls_type_init(&self->ls_type, vm, a_string(vm, "Ls"),
					     A_SUPER(&self->any_type)));
  
  a_lib_bind_type(&self->lib, a_meta_type_init(&self->meta_type, vm, a_string(vm, "Meta"),
					       A_SUPER(&self->any_type)));
  
  a_lib_bind_type(&self->lib, a_func_type_init(&self->func_type, vm, a_string(vm, "Func"),
					       A_SUPER(&self->any_type)));
  
  a_lib_bind_type(&self->lib, a_pair_type_init(&self->pair_type, vm, a_string(vm, "Pair"),
					       A_SUPER(&self->any_type)));

  a_lib_bind_type(&self->lib, a_reg_type_init(&self->reg_type, vm, a_string(vm, "Reg"),
					      A_SUPER(&self->any_type)));
  
  a_lib_bind_type(&self->lib, a_prim_type_init(&self->prim_type, vm, a_string(vm, "Prim"),
					       A_SUPER(&self->any_type)));
  
  a_lib_bind(&self->lib, a_string(vm, "ALISP-VERSION"), &self->int_type)->as_int = A_VERSION;
  a_lib_bind(&self->lib, a_string(vm, "T"), &self->bool_type)->as_bool = true;
  a_lib_bind(&self->lib, a_string(vm, "F"), &self->bool_type)->as_bool = false;

  a_lib_bind_func(&self->lib,
		  a_func(vm, a_string(vm, "="),
			 A_ARG(vm,
			       {a_string(vm, "x"), &vm->abc.any_type},
			       {a_string(vm, "y"), &vm->abc.any_type}),
			 A_RET(vm, &vm->abc.bool_type)))->body = equals_body;

  a_lib_bind_func(&self->lib,
		  a_func(vm, a_string(vm, "<"),
			 A_ARG(vm,
			       {a_string(vm, "x"), &vm->abc.any_type},
			       {a_string(vm, "y"), &vm->abc.any_type}),
			 A_RET(vm, &vm->abc.bool_type)))->body = lt_body;

  a_lib_bind_func(&self->lib,
		  a_func(vm, a_string(vm, ">"),
			 A_ARG(vm,
			       {a_string(vm, "x"), &vm->abc.any_type},
			       {a_string(vm, "y"), &vm->abc.any_type}),
			 A_RET(vm, &vm->abc.bool_type)))->body = gt_body;

  a_lib_bind_prim(&self->lib, a_prim(vm, a_string(vm, "alias"), 2, 2))->body = alias_body;
  a_lib_bind_prim(&self->lib, a_prim(vm, a_string(vm, "d"), 0, 1))->body = d_body;
  a_lib_bind_prim(&self->lib, a_prim(vm, a_string(vm, "do"), 0, -1))->body = do_body;
  a_lib_bind_prim(&self->lib, a_prim(vm, a_string(vm, "if"), 2, 3))->body = if_body;

  a_lib_bind_func(&self->lib,
		  a_func(vm, a_string(vm, "is"),
			 A_ARG(vm,
			       {a_string(vm, "x"), &vm->abc.any_type},
			       {a_string(vm, "y"), &vm->abc.any_type}),
			 A_RET(vm, &vm->abc.bool_type)))->body = is_body;

  a_lib_bind_prim(&self->lib, a_prim(vm, a_string(vm, "let"), 1, -1))->body = let_body;
  a_lib_bind_prim(&self->lib, a_prim(vm, a_string(vm, "reset"), 0, 0))->body = reset_body;
  return self;
}
