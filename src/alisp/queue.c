#include <assert.h>
#include "alisp/fail.h"
#include "alisp/queue.h"
#include "alisp/vm.h"

struct a_queue *a_queue_new(struct a_vm *vm, uint32_t cap) {
  struct a_queue *f = a_ls_null(&vm->free_queues)
    ? a_pool_alloc(&vm->queue_pool)
    : a_baseof(a_ls_pop(vm->free_queues.next), struct a_queue, ls);
  
  return a_queue_init(f, vm, cap);
}

struct a_queue *a_queue_init(struct a_queue *self, struct a_vm *vm, uint32_t cap) {
  self->vm = vm;
  a_ls_init(&self->items);
  self->cap = cap;
  self->len = 0;
  pthread_mutex_init(&self->mutex, NULL);
  pthread_cond_init(&self->popped, NULL);
  pthread_cond_init(&self->pushed, NULL);
  self->ref_count = 1;
  return self;
}

struct a_queue *a_queue_ref(struct a_queue *self) {
  self->ref_count++;
  return self;
}

bool a_queue_deref(struct a_queue *self) {
  assert(self->ref_count);
  if (--self->ref_count) { return false; }
  a_ls_push(self->vm->free_queues.next, &self->ls);
  return true;
}

bool a_queue_push(struct a_queue *self, struct a_val *val) {
  int result = 0;

  if ((result = pthread_mutex_lock(&self->mutex))) {
    a_fail("Failed locking queue: %d", result);
    return false;
  }

  while (self->len == self->cap) {
    if ((result = pthread_cond_wait(&self->popped, &self->mutex))) {
      a_fail("Failed waiting for queue pop: %d", result);
      return false;
    }
  }

  a_ls_push(&self->items, &val->ls);
  self->len++;
  
  if ((result = pthread_mutex_unlock(&self->mutex))) {
    a_fail("Failed unlocking queue: %d", result);
    return false;
  }

  if ((result = pthread_cond_signal(&self->pushed))) {
    a_fail("Failed signalling queue push: %d", result);
    return false;
  }
  
  return true;
}

struct a_val *a_queue_pop(struct a_queue *self) {
  int result = 0;

  if ((result = pthread_mutex_lock(&self->mutex))) {
    a_fail("Failed locking queue: %d", result);
    return false;
  }

  while (!self->len) {
    if ((result = pthread_cond_wait(&self->pushed, &self->mutex))) {
      a_fail("Failed waiting for queue push: %d", result);
      return false;
    }
  }

  struct a_val *v = a_baseof(a_ls_pop(self->items.next), struct a_val, ls);
  v = a_clone(a_val_new(a_type_clone(v->type, self->vm)), v);
  self->len--;
  
  if ((result = pthread_mutex_unlock(&self->mutex))) {
    a_fail("Failed unlocking queue: %d", result);
    return NULL;
  }

  if ((result = pthread_cond_signal(&self->popped))) {
    a_fail("Failed signalling queue push: %d", result);
    return NULL;
  }
  
  return v;
}

