#include <assert.h>
#include <stdio.h>
#include "alisp/string.h"
#include "alisp/vm.h"

struct a_vm *a_vm_init(struct a_vm *self) {
  a_pool_init(&self->pool, NULL, A_PAGE_SIZE / A_MAX_ALIGN, A_MAX_ALIGN);
  a_pool_init(&self->op_pool, &self->pool, A_OP_PAGE_SIZE, sizeof(struct a_op));
  a_pool_init(&self->scope_pool, &self->pool, A_SCOPE_PAGE_SIZE, sizeof(struct a_scope));
  a_pool_init(&self->string_pool, &self->pool, A_STRING_PAGE_SIZE, sizeof(struct a_string) + 10);
  a_pool_init(&self->val_pool, &self->pool, A_VAL_PAGE_SIZE, sizeof(struct a_val));
  a_ls_init(&self->code);
  a_scope_init(&self->main, self, NULL);
  a_ls_init(&self->scopes);
  a_ls_push(&self->scopes, &self->main.vm_scopes);
  a_ls_init(&self->stack);
  self->refs = 1;
  return self;
}

struct a_vm *a_vm_ref(struct a_vm *self) {
  self->refs++;
  return self;
}

bool a_vm_deref(struct a_vm *self) {  
  assert(self->refs);
  if (--self->refs) { return false; }

  a_ls_do(&self->scopes, sls) {
    struct a_scope *s = a_baseof(sls, struct a_scope, vm_scopes);
    a_ls_pop(sls);
    a_scope_deref(s);
    if (s != &self->main) { a_pool_free(&self->scope_pool, s); }
  }

  a_ls_do(&self->stack, vls) {
    struct a_val *v = a_baseof(vls, struct a_val, ls);
    a_ls_pop(vls);
    a_val_deref(v);
    a_pool_free(&self->val_pool, v);
  }

  a_ls_do(&self->code, ols) {
    struct a_op *o = a_baseof(ols, struct a_op, vm_code);
    a_ls_pop(ols);
    a_op_deinit(o);
    a_pool_free(&self->op_pool, o);
  }

  a_pool_deref(&self->val_pool);
  a_pool_deref(&self->string_pool);
  a_pool_deref(&self->scope_pool);
  a_pool_deref(&self->op_pool);
  a_pool_deref(&self->pool);
  return true;
}

a_pc a_vm_pc(struct a_vm *self) { return self->code.next; }

struct a_op *a_vm_emit(struct a_vm *self, enum a_op_type op_type) {
  struct a_op *op = a_op_init(a_pool_malloc(&self->op_pool, sizeof(struct a_op)), op_type);
  a_ls_push(&self->code, &op->vm_code);
  return op;
}

#define A_DISPATCH(prev)						\
  goto *dispatch[a_baseof((pc = prev->next), struct a_op, vm_code)->type]

void a_vm_eval(struct a_vm *self, a_pc pc) {
  static const void* dispatch[] = {&&STOP, &&PUSH};
  A_DISPATCH(pc);

 PUSH: {
    printf("PUSH\n");
    struct a_val
      *src = &a_baseof(pc, struct a_op, vm_code)->as_push.val,
      *dst = a_vm_push(self, src->type);
    a_val_copy(dst, src);
    A_DISPATCH(pc);
  }
  
 STOP: {
    printf("STOP\n");
    // Done!
  }
}

struct a_val *a_vm_push(struct a_vm *self, struct a_type *type) {
  struct a_val *v = a_val_init(a_pool_malloc(&self->val_pool, sizeof(struct a_val)), type);
  a_ls_push(&self->stack, &v->ls);
  return v;
}

struct a_val *a_vm_pop(struct a_vm *self) {
  return a_ls_null(&self->stack) ? NULL : a_baseof(a_ls_pop(self->stack.prev), struct a_val, ls);
}
