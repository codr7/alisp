#include <string.h>
#include "alisp/frame.h"
#include "alisp/func.h"
#include "alisp/scope.h"
#include "alisp/vm.h"

struct a_frame *a_frame_init(struct a_frame *self,
			     struct a_vm *vm,
			     struct a_func *func,
			     enum a_call_flags flags,
			     struct a_func_mem *mem,
			     a_pc_t ret) {
  self->func = func;
  self->flags = flags;
  self->mem = mem;
  self->ret = ret;
  a_reg_t reg = func->min_reg;
  
  for (struct a_val **src = vm->regs + reg, **dst = self->regs + reg; reg <= func->max_reg; reg++, src++, dst++) {
      *dst = *src;
      *src = NULL;
  }
  
  return self;
}

void a_frame_restore(struct a_frame *self, struct a_vm *vm) {
  a_reg_t reg = self->func->min_reg;
  
  for (struct a_val **src = self->regs + reg, **dst = vm->regs + reg; reg <= self->func->max_reg; reg++, src++, dst++) {
    struct a_val *v = self->regs[reg];

    if (*src) {
      if (*dst) {
	a_store(vm, reg, v);
      } else {
	*dst = *src;
      }
    }
  }
}
