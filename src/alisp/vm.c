#include <assert.h>
#include <stdio.h>
#include "alisp/fail.h"
#include "alisp/scope.h"
#include "alisp/string.h"
#include "alisp/vm.h"

struct a_vm *a_vm_init(struct a_vm *self) {
  a_pool_init(&self->pool, NULL, 1, A_PAGE_SIZE);
  a_pool_init(&self->binding_pool, &self->pool, A_BINDING_PAGE_SIZE, sizeof(struct a_binding));
  a_pool_init(&self->op_pool, &self->pool, A_OP_PAGE_SIZE, sizeof(struct a_op));
  a_pool_init(&self->scope_pool, &self->pool, A_SCOPE_PAGE_SIZE, sizeof(struct a_scope));
  a_pool_init(&self->string_pool, &self->pool, A_STRING_PAGE_SIZE, sizeof(struct a_string) + 10);
  a_pool_init(&self->val_pool, &self->pool, A_VAL_PAGE_SIZE, sizeof(struct a_val));
  self->next_type_id = 0;
  a_abc_lib_init(&self->abc, self);
  a_ls_init(&self->code);
  a_scope_init(&self->main, self, NULL);
  a_ls_init(&self->scopes);
  a_ls_push(&self->scopes, &self->main.ls);
  a_ls_init(&self->stack);
  return self;
}

void a_vm_deinit(struct a_vm *self) {  
  a_ls_do(&self->scopes, sls) {
    struct a_scope *s = a_baseof(sls, struct a_scope, ls);
    a_ls_pop(sls);
    a_scope_deref(s);
    if (s != &self->main) { a_free(&self->scope_pool, s); }
  }

  a_ls_do(&self->stack, vls) {
    struct a_val *v = a_baseof(vls, struct a_val, ls);
    a_ls_pop(vls);
    a_val_deref(v);
    a_free(&self->val_pool, v);
  }

  a_ls_do(&self->code, ols) {
    struct a_op *o = a_baseof(ols, struct a_op, ls);
    a_ls_pop(ols);
    a_op_deinit(o);
    a_free(&self->op_pool, o);
  }

  a_pool_deref(&self->val_pool);
  a_pool_deref(&self->string_pool);
  a_pool_deref(&self->scope_pool);
  a_pool_deref(&self->op_pool);
  a_pool_deref(&self->binding_pool);
  a_pool_deref(&self->pool);
}

a_pc a_next_pc(struct a_vm *self) { return self->code.next; }

struct a_op *a_emit(struct a_vm *self, enum a_op_type op_type) {
  struct a_op *op = a_op_init(a_malloc(&self->op_pool, sizeof(struct a_op)), op_type);
  a_ls_push(&self->code, &op->ls);
  return op;
}

#define A_DISPATCH(prev)						\
  goto *dispatch[a_baseof((pc = prev->next), struct a_op, ls)->type]

void a_eval(struct a_vm *self, a_pc pc) {
  static const void* dispatch[] = {&&STOP, &&CALL, &&LOAD, &&PUSH, &&STORE};
  A_DISPATCH(pc);

 CALL: {
    printf("CALL\n");
    struct a_val *t = &a_baseof(pc, struct a_op, ls)->as_call.target;
    A_DISPATCH(a_call(t, pc, true));
  }
  
 LOAD: {
    printf("LOAD\n");
    struct a_val *v = self->regs + a_baseof(pc, struct a_op, ls)->as_load.reg;
    a_copy(a_push(self, v->type), v);
    A_DISPATCH(pc);
  }
  
 PUSH: {
    printf("PUSH\n");
    struct a_val
      *src = &a_baseof(pc, struct a_op, ls)->as_push.val,
      *dst = a_push(self, src->type);
    a_copy(dst, src);
    A_DISPATCH(pc);
  }

 STORE: {
    printf("STORE\n");
    struct a_val *v = a_pop(self);
    if (!v) { a_fail("Missing value to store"); }
    self->regs[a_baseof(pc, struct a_op, ls)->as_load.reg] = *v;
    a_free(&self->val_pool, v);
    A_DISPATCH(pc);
  }
  
 STOP: {
    printf("STOP\n");
    // Done!
  }
}

struct a_scope *a_scope(struct a_vm *self) {
  return a_baseof(self->scopes.next, struct a_scope, ls);
}

struct a_val *a_push(struct a_vm *self, struct a_type *type) {
  struct a_val *v = a_val_init(a_malloc(&self->val_pool, sizeof(struct a_val)), type);
  a_ls_push(&self->stack, &v->ls);
  return v;
}

struct a_val *a_pop(struct a_vm *self) {
  return a_ls_null(&self->stack) ? NULL : a_baseof(a_ls_pop(self->stack.prev), struct a_val, ls);
}

a_reg a_bind_reg(struct a_vm *self, struct a_string *key) {
  struct a_scope *s = a_scope(self);
  a_reg r = s->next_reg++;
  a_scope_bind(s, key, &self->abc.reg_type)->as_reg = r;
  return r;
}
