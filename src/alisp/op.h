#ifndef ALISP_OP_H
#define ALISP_OP_H

#include "alisp/ls.h"
#include "alisp/val.h"

enum a_op_type {
  A_STOP_OP=0,
  A_BENCH_OP, A_BRANCH_OP,
  A_CALL_OP, 
  A_DROP_OP, A_DUP_OP,
  A_FENCE_OP, A_FOR_OP,
  A_GOTO_OP, A_LOAD_OP, A_PUSH_OP,
  A_RESET_OP, A_RET_OP,
  A_STORE_OP, A_SWAP_OP,
  A_TEST_OP, A_THREAD_OP,
  A_ZIP_OP};

struct a_bench_op {
  int reps;
  a_pc_t end_pc;
};

struct a_branch_op {
  a_pc_t right_pc;
};
  
struct a_call_op {
  struct a_val *target;
  enum a_call_flags flags;
};

struct a_drop_op {
  int8_t offset, count;
};

struct a_dup_op {
  int8_t offset;
};

struct a_for_op {
  a_pc_t end_pc;
};

struct a_goto_op {
  a_pc_t pc;
};

struct a_load_op {
  a_reg_t reg;
  struct a_type *type;
};

struct a_push_op {
  struct a_val val;
};

struct a_ret_op {
  struct a_func *func;
  bool check;
};

struct a_store_op {
  a_reg_t reg;
};

struct a_swap_op {
  int8_t offset;
};

struct a_test_op {
  struct a_string *desc;
  struct a_ls stack;
  a_pc_t end_pc;
};

struct a_thread_op {
  struct a_ls args;
};

struct a_op {
  struct a_ls pc;
  enum a_op_type type;
  
  union {
    struct a_bench_op as_bench;
    struct a_branch_op as_branch;
    struct a_call_op as_call;
    struct a_drop_op as_drop;
    struct a_dup_op as_dup;
    struct a_for_op as_for;
    struct a_goto_op as_goto;
    struct a_load_op as_load;
    struct a_push_op as_push;
    struct a_ret_op as_ret;
    struct a_store_op as_store;
    struct a_swap_op as_swap;
    struct a_test_op as_test;
    struct a_thread_op as_thread;
  };
};

struct a_op *a_op_init(struct a_op *self, enum a_op_type type);
a_pc_t a_op_analyze(struct a_op *self, struct a_vm *vm);

#endif
