#ifndef ALISP_OP_H
#define ALISP_OP_H

#include "alisp/ls.h"
#include "alisp/val.h"

enum a_op_type {A_STOP_OP=0, A_CALL_OP, A_LOAD_OP, A_PUSH_OP, A_STORE_OP};

struct a_call_op {
  struct a_val *target;
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
    struct a_call_op as_call;
    struct a_load_op as_load;
    struct a_push_op as_push;
    struct a_store_op as_store;
  };
};

struct a_op *a_op_init(struct a_op *self, enum a_op_type type);
void a_op_deinit(struct a_op *self);

#endif
