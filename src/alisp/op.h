#ifndef ALISP_OP_H
#define ALISP_OP_H

#include "alisp/ls.h"
#include "alisp/val.h"

enum a_op_type {
  A_STOP_OP=0,
  A_BRANCH_OP,
  A_CALL_OP, A_COPY_OP,
  A_DROP_OP, A_GOTO_OP, A_LOAD_OP, A_PUSH_OP, A_RESET_OP, A_STORE_OP, A_ZIP_OP};

struct a_branch_op {
  a_pc right_pc;
};
  
struct a_call_op {
  struct a_val *target;
  enum a_call_flags flags;
};

struct a_copy_op {
  int8_t offset;
};

struct a_drop_op {
  int8_t count;
};

struct a_goto_op {
  a_pc pc;
};

struct a_load_op {
  a_reg reg;
};

struct a_push_op {
  struct a_val val;
};

struct a_store_op {
  a_reg reg;
};

struct a_op {
  struct a_ls ls;
  enum a_op_type type;
  
  union {
    struct a_branch_op as_branch;
    struct a_call_op as_call;
    struct a_copy_op as_copy;
    struct a_drop_op as_drop;
    struct a_goto_op as_goto;
    struct a_load_op as_load;
    struct a_push_op as_push;
    struct a_store_op as_store;
  };
};

struct a_op *a_op_init(struct a_op *self, enum a_op_type type);
void a_op_deinit(struct a_op *self);

#endif
