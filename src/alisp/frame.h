#ifndef ALISP_FRAME_H
#define ALISP_FRAME_H

#include "alisp/ls.h"
#include "alisp/types.h"

struct a_vm;

struct a_frame {
  struct a_ls ls;
  struct a_val *regs[A_REG_COUNT];
  a_pc ret;
};

struct a_frame *a_frame_init(struct a_frame *self, struct a_vm *vm, a_pc ret);
void a_frame_deinit(struct a_frame *self);
a_pc a_frame_restore(struct a_frame *self, struct a_vm *vm);

#endif
