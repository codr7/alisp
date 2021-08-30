#include "alisp/frame.h"

struct a_frame *a_frame_init(struct a_frame *self,
			     struct a_vm *vm,
			     struct a_func *func,
			     enum a_call_flags flags,
			     a_pc_t ret) {
  self->func = func;
  self->flags = flags;
  self->ret = ret;
  return self;
}

void a_frame_deinit(struct a_frame *self) {

}

a_pc_t a_frame_restore(struct a_frame *self, struct a_vm *vm) {
  return self->ret;
}
