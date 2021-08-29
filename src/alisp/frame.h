#ifndef ALISP_FRAME_H
#define ALISP_FRAME_H

#include "alisp/ls.h"
#include "alisp/types.h"

struct a_vm;

struct a_frame {
  struct a_ls ls;
  struct a_func *func;
  a_pc_t ret;
};

struct a_frame *a_frame_init(struct a_frame *self, struct a_vm *vm, struct a_func *func, a_pc_t ret);
void a_frame_deinit(struct a_frame *self);
a_pc_t a_frame_restore(struct a_frame *self, struct a_vm *vm);

#endif
