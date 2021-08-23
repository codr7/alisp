#ifndef ALISP_OP_H
#define ALISP_OP_H

#include "alisp/ls.h"
#include "alisp/val.h"

enum a_op_type {A_STOP=0, A_CALL, A_LOAD, A_PUSH, A_STORE};

struct a_call {
  struct a_val target;
};

struct a_load {
  a_reg reg;
};

struct a_push {
  struct a_val val;
};

struct a_store {
  a_reg reg;
};

struct a_op {
  struct a_ls ls;
  enum a_op_type type;
  
  union {
    struct a_call as_call;
    struct a_load as_load;
    struct a_push as_push;
    struct a_store as_store;
  };
};

struct a_op *a_op_init(struct a_op *self, enum a_op_type type);
void a_op_deinit(struct a_op *self);

#endif
