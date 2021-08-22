#include <assert.h>
#include "alisp/scope.h"
#include "alisp/string.h"
#include "alisp/val.h"

struct a_binding {
  struct a_ls set_items;
  struct a_string *key;
  struct a_val val;
};

static enum a_order compare_bindings(const struct a_ls *x, const struct a_ls *y) {
  return a_string_compare(a_baseof(x, struct a_binding, set_items)->key,
			  a_baseof(y, struct a_binding, set_items)->key);
}

struct a_scope *a_scope_init(struct a_scope *self, struct a_vm *vm, struct a_scope *outer) {
  self->vm = vm;
  self->outer = outer ? a_scope_ref(outer) : NULL;
  a_ls_init(&self->vm_scopes);
  a_lset_init(&self->bindings, compare_bindings);
  self->next_register = 0;
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
