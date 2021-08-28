#include "alisp/binding.h"
#include "alisp/fail.h"
#include "alisp/func.h"
#include "alisp/lib.h"
#include "alisp/prim.h"
#include "alisp/pool.h"
#include "alisp/string.h"
#include "alisp/vm.h"

struct a_lib *a_lib_init(struct a_lib *self, struct a_vm *vm, struct a_string *name) {
  self->vm = vm;
  a_lset_init(&self->bindings, a_binding_key, a_binding_compare);
  return self;
}

void a_lib_deinit(struct a_lib *self) {
  a_ls_do(&self->bindings.items, bls) {
    a_binding_deinit(a_baseof(a_baseof(bls, struct a_val, ls), struct a_binding, val));
  }
}

struct a_val *a_lib_bind(struct a_lib *self, struct a_string *key, struct a_type *type) {
  struct a_binding *b = a_malloc(&self->vm->binding_pool, sizeof(struct a_binding));
  b->key = key;

  if (!a_lset_insert(&self->bindings, &b->val.ls, false)) {
    a_free(&self->vm->binding_pool, b);
    return NULL;
  }
  
  return a_val_init(&b->val, type);
}

struct a_func *a_lib_bind_func(struct a_lib *self, struct a_func *func) {
  struct a_val *v = a_lib_bind(self, func->name, &self->vm->abc.func_type);
  if (!v) { return NULL; }
  v->as_func = func;
  return func;
}

struct a_prim *a_lib_bind_prim(struct a_lib *self, struct a_prim *prim) {
  struct a_val *v = a_lib_bind(self, prim->name, &self->vm->abc.prim_type);
  if (!v) { return NULL; }
  v->as_prim = prim;
  return prim;
}

struct a_type *a_lib_bind_type(struct a_lib *self, struct a_type *type) {
  a_lib_bind(self, type->name, &self->vm->abc.meta_type)->as_meta = type;
  return type;
}

bool a_lib_import(struct a_lib *self) {
  struct a_scope *s = a_scope(self->vm);

  a_ls_do(&self->bindings.items, ls) {
    struct a_binding *src = a_baseof(a_baseof(ls, struct a_val, ls), struct a_binding, val);
    struct a_val *dst = a_scope_bind(s, src->key, src->val.type);

    if (!dst) {
      a_fail("Failed importing: %s", src->key->data);
      return false;
    }

    a_copy(dst, &src->val);
  }

  return true;
}
