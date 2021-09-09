#include <assert.h>
#include "alisp/iter.h"
#include "alisp/vm.h"

struct a_iter *a_iter_new(struct a_vm *vm, a_iter_body_t body) {
  a_ls_do(&vm->free_iters, ls) {
    return a_iter_init(a_baseof(a_ls_pop(ls), struct a_iter, ls), body);
  }

  return a_iter_init(a_pool_alloc(&vm->iter_pool), body);
}

struct a_iter *a_iter_init(struct a_iter *self, a_iter_body_t body) {
  a_ls_init(&self->data);
  self->body = body;
  return self;
}

struct a_iter *a_iter_ref(struct a_iter *self) {
  self->ref_count++;
  return self;
}

bool a_iter_deref(struct a_iter *self, struct a_vm *vm) {
  assert(self->ref_count);
  if (--self->ref_count) { return false; }  
  a_ls_do(&self->data, ls) { a_val_free(a_baseof(ls, struct a_val, ls), vm); }
  a_ls_push(vm->free_iters.next, &self->ls);
  return true;
}

struct a_val *a_iter_next(struct a_iter *self, struct a_vm *vm) {
  assert(self->body);
  return self->body(self, vm);
}
