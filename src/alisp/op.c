#include "alisp/op.h"

struct a_op *a_op_init(struct a_op *self, enum a_op_type type) {
  self->type = type;

  switch (self->type) {
  case A_CALL_OP:
    self->as_call.flags = 0;
    break;
  case A_BENCH_OP:
  case A_BRANCH_OP:
  case A_COPY_OP:
    self->as_copy.offset = 0;
    break;
  case A_DROP_OP:
    self->as_drop.count = 1;
    break;
  case A_GOTO_OP:
  case A_LOAD_OP:
  case A_PUSH_OP:
  case A_RESET_OP:
  case A_RET_OP:
  case A_STOP_OP:
  case A_STORE_OP:
  case A_ZIP_OP:
    break;
  }

  return self;
}

void a_op_deinit(struct a_op *self) {
  switch (self->type) {
  case A_CALL_OP: {
    struct a_val *t = self->as_call.target;
    if (t) { a_val_deref(t); }
    break;
  }
  case A_PUSH_OP:
    a_val_deref(&self->as_push.val);
    break;
  case A_BENCH_OP:
  case A_BRANCH_OP:
  case A_COPY_OP:
  case A_DROP_OP:
  case A_GOTO_OP:
  case A_LOAD_OP:
  case A_STOP_OP:
  case A_RESET_OP:
  case A_RET_OP:
  case A_STORE_OP:
  case A_ZIP_OP:
    break;
  }
}
