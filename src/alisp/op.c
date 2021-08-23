#include "alisp/op.h"

struct a_op *a_op_init(struct a_op *self, enum a_op_type type) {
  self->type = type;
  return self;
}

void a_op_deinit(struct a_op *self) {
  switch (self->type) {
  case A_PUSH:
    a_val_deref(&self->as_push.val);
    break;
  case A_LOAD:
  case A_STOP:
  case A_STORE:
    break;
  }
}
