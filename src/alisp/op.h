#ifndef ALISP_OP_H
#define ALISP_OP_H

#include "alisp/ls.h"
#include "alisp/val.h"

enum a_op_type {A_STOP=0, A_PUSH};

struct a_push {
  struct a_val val;
};

struct a_op {
  enum a_op_type type;
  struct a_ls vm_code;
  
  union {
    struct a_push as_push;
  };
};

struct a_op *a_op_init(struct a_op *self, enum a_op_type type);
void a_op_deinit(struct a_op *self);

#endif
