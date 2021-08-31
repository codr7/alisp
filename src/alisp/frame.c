#include <string.h>
#include "alisp/frame.h"
#include "alisp/func.h"
#include "alisp/scope.h"
#include "alisp/vm.h"

struct a_frame *a_frame_init(struct a_frame *self,
			     struct a_vm *vm,
			     struct a_func *func,
			     enum a_call_flags flags,
			     a_pc_t ret) {
  self->func = func;
  self->flags = flags;
  self->ret = ret;

  for (a_reg_t *reg = func->regs; reg < func->regs + func->reg_count; reg++) {
    struct a_val *src = vm->regs[*reg];
    
    if (src) {
      struct a_val *dst = a_malloc(vm, sizeof(struct a_val));
      self->regs[*reg] = dst;
      a_copy(a_val_init(dst, src->type), src);
    }
  }
  
  return self;
}

a_pc_t a_frame_restore(struct a_frame *self, struct a_vm *vm) {
  for (a_reg_t *reg = self->func->regs; reg < self->func->regs + self->func->reg_count; reg++) {
    struct a_val *v = self->regs[*reg];
    if (v) { a_store(vm, *reg, v); }
  }
  
  return self->ret;
}
