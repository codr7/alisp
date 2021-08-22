#include <assert.h>
#include "alisp/vm.h"

struct a_vm *a_vm_init(struct a_vm *self) {
  a_pool_init(&self->pool, NULL, A_PAGE_SIZE / A_MAX_ALIGN, A_MAX_ALIGN);
  a_pool_init(&self->scope_pool, &self->pool, A_SCOPE_PAGE_SIZE, sizeof(struct a_scope));
  a_scope_init(&self->main, self, NULL);
  a_ls_init(&self->scopes);
  a_ls_append(&self->scopes, &self->main.vm_scopes);
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

  a_pool_deref(&self->scope_pool);

  a_ls_do(&self->scopes, sls) {
    struct a_scope *s = a_baseof(sls, struct a_scope, vm_scopes);
    a_scope_deref(s);
    if (s != &self->main) { a_pool_free(&self->scope_pool, s); }
  }

  return true;
}
