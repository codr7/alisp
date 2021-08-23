#include <assert.h>
#include "alisp/pool.h"
#include "alisp/scope.h"
#include "alisp/string.h"
#include "alisp/val.h"
#include "alisp/vm.h"

static const void *binding_key(const struct a_ls *x) {
  return a_baseof(x, struct a_binding, ls)->key;
}

static enum a_order compare_bindings(const void *x, const void *y) {
  return a_string_compare(x, y);
}

struct a_scope *a_scope_init(struct a_scope *self, struct a_vm *vm, struct a_scope *outer) {
  self->vm = vm;
  self->outer = outer ? a_scope_ref(outer) : NULL;
  a_ls_init(&self->ls);
  a_lset_init(&self->bindings, binding_key, compare_bindings);
  self->next_reg = 0;
  self->ref_count = 1;
  return self;
}

struct a_scope *a_scope_ref(struct a_scope *self) {
  self->ref_count++;
  return self;
}

bool a_scope_deref(struct a_scope *self) {
  assert(self->ref_count);
  if (--self->ref_count) { return false; }
  if (self->outer) { a_scope_deref(self->outer); }

  a_ls_do(&self->bindings.items, bls) {
    struct a_binding *b = a_baseof(bls, struct a_binding, ls);
    a_string_deref(b->key);
    a_val_deref(&b->val);
  }
  
  return true;
}

struct a_val *a_scope_bind(struct a_scope *self, struct a_string *key, struct a_type *type) {
  struct a_binding *b = a_malloc(&self->vm->binding_pool, sizeof(struct a_binding));
  b->key = a_string_ref(key);
  return a_val_init(&b->val, type);
}

struct a_val *a_scope_find(struct a_scope *self, const struct a_string *key) {
  struct a_ls *found = a_lset_find(&self->bindings, key);
  if (!found) { return NULL; }
  return a_baseof(found, struct a_val, ls);
}
