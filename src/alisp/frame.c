#include "alisp/frame.h"

struct a_frame *a_frame_init(struct a_frame *self, struct a_vm *vm, a_pc_t ret) {
  self->ret = ret;
  return self;
}

void a_frame_deinit(struct a_frame *self) {

}

a_pc_t a_frame_restore(struct a_frame *self, struct a_vm *vm) {
  return self->ret;
}
