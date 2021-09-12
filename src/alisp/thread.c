#include <assert.h>
#include "alisp/fail.h"
#include "alisp/form.h"
#include "alisp/stack.h"
#include "alisp/thread.h"

struct a_thread *a_thread_new(struct a_vm *owner, struct a_rets rets) {
  struct a_thread *f = a_ls_null(&owner->free_threads)
    ? a_pool_alloc(&owner->thread_pool)
    : a_baseof(a_ls_pop(owner->free_threads.next), struct a_thread, ls);
  
  return a_thread_init(f, owner, rets);
}

struct a_thread *a_thread_init(struct a_thread *self, struct a_vm *owner, struct a_rets rets) {
  self->owner = owner;
  a_vm_init(&self->vm);
  self->rets = rets;
  self->ref_count = 1;
  return self;
}

struct a_thread *a_thread_ref(struct a_thread *self) {
  self->ref_count++;
  return self;
}

bool a_thread_deref(struct a_thread *self) {
  assert(self->ref_count);
  if (--self->ref_count) { return false; }  
  a_vm_deinit(&self->vm);
  a_ls_push(self->owner->free_threads.next, &self->ls);
  return true;
}

bool a_thread_emit(struct a_thread *self, struct a_ls *args) {
  a_ls_do(args, ls) {
    struct a_form *f = a_form_clone(a_baseof(ls, struct a_form, ls), &self->vm, self->owner);
    if (!a_form_emit(f, &self->vm)) { return false; }
    a_form_deref(f, &self->vm);
  }

  a_emit(&self->vm, A_STOP_OP);
  a_pc_t pc = &self->vm.code;
  return a_analyze(&self->vm, pc);
}

static void *body(void *data) {
  struct a_thread *self = data;
  a_pc_t pc = &self->vm.code;
  a_eval(&self->vm, pc);
  return NULL;
}

bool a_thread_start(struct a_thread *self) {
  int err = pthread_create(&self->handle, NULL, body, self);

  if (err) {
    a_fail("Failed starting thread: %d", err);
    return false;
  }

  return true;
}

bool a_thread_join(struct a_thread *self) {
  int err = pthread_join(self->handle, NULL);

  if (err) {
    a_fail("Failed joining thread: %d", err);
    return false;
  }

  a_ls_do(&self->vm.stack, ls) {
    struct a_val *v = a_baseof(ls, struct a_val, ls);
    a_clone(a_push(self->owner, a_type_clone(v->type, self->owner)), v);
  }

  return true;
}
