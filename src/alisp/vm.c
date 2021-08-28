#include <assert.h>
#include <stdio.h>
#include "alisp/fail.h"
#include "alisp/form.h"
#include "alisp/func.h"
#include "alisp/prim.h"
#include "alisp/scope.h"
#include "alisp/stack.h"
#include "alisp/string.h"
#include "alisp/vm.h"

struct a_vm *a_vm_init(struct a_vm *self) {
  a_pool_init(&self->pool, NULL, 1, A_PAGE_SIZE);
  a_pool_init(&self->binding_pool, &self->pool, A_BINDING_PAGE_SIZE, sizeof(struct a_binding));
  a_pool_init(&self->form_pool, &self->pool, A_FORM_PAGE_SIZE, sizeof(struct a_form));
  a_pool_init(&self->func_pool, &self->pool, A_FUNC_PAGE_SIZE, sizeof(struct a_func));
  a_pool_init(&self->ls_pool, &self->pool, A_LS_PAGE_SIZE, sizeof(struct a_ls));
  a_pool_init(&self->op_pool, &self->pool, A_OP_PAGE_SIZE, sizeof(struct a_op));
  a_pool_init(&self->prim_pool, &self->pool, A_PRIM_PAGE_SIZE, sizeof(struct a_prim));
  a_pool_init(&self->scope_pool, &self->pool, A_SCOPE_PAGE_SIZE, sizeof(struct a_scope));
  a_pool_init(&self->string_pool, &self->pool, A_STRING_PAGE_SIZE, sizeof(struct a_string) + 10);
  a_pool_init(&self->val_pool, &self->pool, A_VAL_PAGE_SIZE, sizeof(struct a_val));
  self->next_type_id = 0;
  a_ls_init(&self->code);
  a_scope_init(&self->main, self, NULL);
  a_ls_init(&self->scopes);
  a_ls_push(&self->scopes, &self->main.ls);
  a_ls_init(&self->stack);
  a_abc_lib_init(&self->abc, self);
  a_lib_import(&self->abc.lib);
  a_math_lib_init(&self->math, self);
  a_lib_import(&self->math.lib);
  return self;
}

void a_vm_deinit(struct a_vm *self) {  
  a_ls_do(&self->scopes, sls) {
    struct a_scope *s = a_baseof(sls, struct a_scope, ls);
    a_ls_pop(sls);
    a_scope_deref(s);
  }

  a_ls_do(&self->stack, vls) {
    struct a_val *v = a_baseof(vls, struct a_val, ls);
    a_ls_pop(vls);
    a_val_deref(v);
  }

  a_ls_do(&self->code, ols) {
    struct a_op *o = a_baseof(ols, struct a_op, ls);
    a_ls_pop(ols);
    a_op_deinit(o);
  }

  a_lib_deinit(&self->math.lib);
  a_lib_deinit(&self->abc.lib);
  
  a_pool_deref(&self->val_pool);
  a_pool_deref(&self->string_pool);
  a_pool_deref(&self->scope_pool);
  a_pool_deref(&self->prim_pool);
  a_pool_deref(&self->op_pool);
  a_pool_deref(&self->ls_pool);
  a_pool_deref(&self->func_pool);
  a_pool_deref(&self->form_pool);
  a_pool_deref(&self->binding_pool);
  a_pool_deref(&self->pool);
}

a_pc a_next_pc(struct a_vm *self) { return self->code.prev; }

struct a_op *a_emit(struct a_vm *self, enum a_op_type op_type) {
  struct a_op *op = a_op_init(a_malloc(&self->op_pool, sizeof(struct a_op)), op_type);
  a_ls_push(&self->code, &op->ls);
  return op;
}

struct a_scope *a_scope(struct a_vm *self) {
  return a_baseof(self->scopes.next, struct a_scope, ls);
}

a_reg a_bind_reg(struct a_vm *self, struct a_string *key) {
  struct a_scope *s = a_scope(self);
  a_reg r = s->next_reg++;
  a_scope_bind(s, key, &self->abc.reg_type)->as_reg = r;
  return r;
}
