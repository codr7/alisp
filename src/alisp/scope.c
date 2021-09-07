#include <assert.h>
#include "alisp/fail.h"
#include "alisp/scope.h"
#include "alisp/string.h"
#include "alisp/val.h"
#include "alisp/vm.h"

struct a_scope *a_scope_init(struct a_scope *self, struct a_vm *vm, struct a_scope *outer) {
  self->vm = vm;
  self->outer = outer ? a_scope_ref(outer) : NULL;
  a_ls_init(&self->ls);
  a_lset_init(&self->bindings, a_binding_key, a_binding_compare);
  self->next_reg = outer ? outer->next_reg : 0;
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
    a_binding_deinit(a_baseof(a_baseof(bls, struct a_val, ls), struct a_binding, val));
  }

  return true;
}

struct a_val *a_scope_bind(struct a_scope *self, struct a_string *key, struct a_type *type) {
  struct a_binding *b = a_malloc(self->vm, sizeof(struct a_binding));
  b->key = key;

  if (!a_lset_add(&self->bindings, &b->val.ls, false)) {
    a_free(self->vm, b);
    return NULL;
  }
  
  return a_val_init(&b->val, type);
}

a_reg_t a_scope_bind_reg(struct a_scope *self, struct a_string *key) {
  a_check(self->next_reg < A_REG_COUNT-1, "Maximum number of registers exceeded");
  a_reg_t r = self->next_reg++;
  a_scope_bind(self, key, &self->vm->abc.reg_type)->as_reg = r;
  return r;
}


bool a_scope_unbind(struct a_scope *self, struct a_string *key) {
  struct a_val *v = a_scope_find(self, key);
  if (!v) { return false; }
  a_ls_pop(&v->ls);
  a_val_free(v, self->vm);
  return true;
}

struct a_val *a_scope_find(struct a_scope *self, const struct a_string *key) {
  struct a_ls *found = a_lset_find(&self->bindings, key);
  if (!found) { return self->outer ? a_scope_find(self->outer, key) : NULL; }
  return a_baseof(found, struct a_val, ls);
}
