#ifndef ALISP_ITER_H
#define ALISP_ITER_H

#include "alisp/ls.h"
#include "alisp/types.h"

struct a_iter;
struct a_val;
struct a_vm;

typedef struct a_val *(*a_iter_body_t)(struct a_iter *self, struct a_vm *vm);

struct a_iter {
  struct a_ls ls;
  struct a_ls data;
  a_iter_body_t body;
  a_ref_count_t ref_count;
};

struct a_iter *a_iter_new(struct a_vm *vm, a_iter_body_t body);
struct a_iter *a_iter_init(struct a_iter *self, a_iter_body_t body);
struct a_iter *a_iter_ref(struct a_iter *self);
bool a_iter_deref(struct a_iter *self, struct a_vm *vm);
struct a_val *a_iter_next(struct a_iter *self, struct a_vm *vm);

#endif
