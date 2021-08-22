#include <assert.h>
#include "alisp/scope.h"

struct a_scope *a_scope_init(struct a_scope *self, struct a_vm *vm, struct a_scope *outer) {
  self->vm = vm;
  self->outer = outer ? a_scope_ref(outer) : NULL;
  a_ls_init(&self->vm_scopes);
  self->refs = 1;
  return self;
}

struct a_scope *a_scope_ref(struct a_scope *self) {
  self->refs++;
  return self;
}

bool a_scope_deref(struct a_scope *self) {
  assert(self->refs);
  if (--self->refs) { return false; }
  if (self->outer) { a_scope_deref(self->outer); }
  return true;
}
