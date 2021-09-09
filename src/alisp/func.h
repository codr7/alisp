#ifndef ALISP_FUNC_H
#define ALISP_FUNC_H

#include <stdbool.h>
#include <string.h>
#include "alisp/limits.h"
#include "alisp/lset.h"
#include "alisp/types.h"

#define A_ARG(vm, ...)							\
  ({									\
    struct a_arg items[] = {__VA_ARGS__};				\
    struct a_args args;							\
    args.count = sizeof(items) / sizeof(struct a_arg);			\
    memcpy(args.items, items, sizeof(items));				\
    args;								\
  })

#define A_RET(vm, ...)							\
  ({									\
    struct a_type *items[] = {__VA_ARGS__};				\
    struct a_rets rets;							\
    rets.count = sizeof(items) / sizeof(struct a_type *);		\
    memcpy(rets.items, items, sizeof(items));				\
    rets;								\
  })

struct a_vm;

struct a_arg {
  struct a_string *name;
  struct a_type *type;
  a_reg_t reg;
};

struct a_args {
  uint8_t count;
  struct a_arg items[A_MAX_ARGS];
};

struct a_rets {
  uint8_t count;
  struct a_type *items[A_MAX_RETS];
};

typedef a_type_id_t a_func_weight_t;

struct a_func {
  struct a_ls ls;
  struct a_string *name;
  struct a_args args;
  struct a_rets rets;
  a_func_weight_t weight;
  struct a_lset mem;
  a_pc_t start_pc;
  struct a_scope *scope;
  a_reg_t regs[A_REG_COUNT];
  uint16_t reg_count;
  a_pc_t (*body)(struct a_func *self, struct a_vm *vm, a_pc_t ret);
};

struct a_func_mem {
  struct a_ls ls;
  struct a_ls args;
  struct a_ls rets;
};

struct a_func *a_func_new(struct a_vm *vm,
			  struct a_string *name,
			  struct a_args args,
			  struct a_rets rets);

struct a_func *a_func_init(struct a_func *self,
			   struct a_string *name,
			   struct a_args args,
			   struct a_rets rets);

void a_func_begin(struct a_func *self, struct a_vm *vm);
void a_func_end(struct a_func *self, struct a_vm *vm);

bool a_func_applicable(struct a_func *self, struct a_vm *vm);
a_pc_t a_func_call(struct a_func *self, struct a_vm *vm, enum a_call_flags flags, a_pc_t ret);
void a_func_mem(struct a_func *self, struct a_vm *vm, struct a_func_mem *mem);

#endif
