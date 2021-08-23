#include "alisp/binding.h"
#include "alisp/lib.h"
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
  b->key = a_string_ref(key);

  if (!a_lset_insert(&self->bindings, &b->val.ls, false)) {
    a_free(&self->vm->binding_pool, b);
    return NULL;
  }
  
  return a_val_init(&b->val, type);
}

struct a_type *a_lib_bind_type(struct a_lib *self, struct a_type *type) {
  a_lib_bind(self, type->name, &self->vm->abc.meta_type)->as_meta = type;
  return type;
}
