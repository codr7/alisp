#ifndef ALISP_QUEUE_H
#define ALISP_QUEUE_H

#include <inttypes.h>
#include <pthread.h>
#include "alisp/ls.h"
#include "alisp/types.h"

struct a_val;
struct a_vm;

struct a_queue {
  struct a_ls ls;
  struct a_vm *vm;
  struct a_ls items;
  uint32_t cap, len;
  
  pthread_mutex_t mutex;
  pthread_cond_t popped, pushed;
  
  a_ref_count_t ref_count;
};

struct a_queue *a_queue_new(struct a_vm *vm, uint32_t cap);
struct a_queue *a_queue_init(struct a_queue *self, struct a_vm *vm, uint32_t cap);
struct a_queue *a_queue_ref(struct a_queue *self);
bool a_queue_deref(struct a_queue *self);

bool a_queue_push(struct a_queue *self, struct a_val *val);
struct a_val *a_queue_pop(struct a_queue *self);

#endif
