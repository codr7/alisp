#ifndef ALISP_THREAD_H
#define ALISP_THREAD_H

#include <pthread.h>
#include "alisp/vm.h"

struct a_thread {
  struct a_ls ls;
  struct a_vm *owner, vm;
  pthread_t handle;
  a_ref_count_t ref_count;
};

struct a_thread *a_thread_new(struct a_vm *owner);
struct a_thread *a_thread_init(struct a_thread *self, struct a_vm *owner);
struct a_thread *a_thread_ref(struct a_thread *self);
bool a_thread_deref(struct a_thread *self);

bool a_thread_emit(struct a_thread *self, struct a_ls *args);
bool a_thread_start(struct a_thread *self);

#endif
