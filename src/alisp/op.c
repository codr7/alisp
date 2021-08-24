#include "alisp/op.h"

struct a_op *a_op_init(struct a_op *self, enum a_op_type type) {
  self->type = type;
  return self;
}

void a_op_deinit(struct a_op *self) {
  switch (self->type) {
  case A_CALL_OP:
    a_val_deref(&self->as_call.target);
    break;
  case A_PUSH_OP:
    a_val_deref(&self->as_push.val);
    break;
  case A_LOAD_OP:
  case A_STOP_OP:
  case A_STORE_OP:
    break;
  }
}
