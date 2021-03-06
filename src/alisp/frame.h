#ifndef ALISP_FRAME_H
#define ALISP_FRAME_H

#include "alisp/limits.h"
#include "alisp/ls.h"
#include "alisp/types.h"

struct a_vm;

struct a_frame {
  struct a_ls ls;
  struct a_func *func;
  enum a_call_flags flags;
  struct a_func_mem *mem;
  struct a_val *regs[A_REG_COUNT];
  a_pc_t ret;
};

struct a_frame *a_frame_init(struct a_frame *self,
			     struct a_vm *vm,
			     struct a_func *func,
			     enum a_call_flags flags,
			     struct a_func_mem *mem,
			     a_pc_t ret);

void a_frame_restore(struct a_frame *self, struct a_vm *vm);

#endif
