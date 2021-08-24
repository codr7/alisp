#ifndef ALISP_FUNC_H
#define ALISP_FUNC_H

#include <stdbool.h>
#include "alisp/types.h"

struct a_func {
  struct a_vm *vm;
  struct a_string *name;
  uint8_t arg_count, ret_count;
  struct a_arg *args;
  struct a_type **rets;
  a_pc (*body)(struct a_func *self, a_pc ret);
  a_ref_count ref_count;
};

struct a_arg {
  struct a_string *name;
  struct a_type *type;
};

struct a_func *a_func_init(struct a_func *self,
			   struct a_vm *vm,
			   struct a_string *name,
			   int arg_count, struct a_arg args[],
			   struct a_type *rets[]);

struct a_func *a_func_ref(struct a_func *self);
bool a_func_deref(struct a_func *self);
bool a_func_applicable(struct a_func *self);
a_pc a_func_call(struct a_func *self, a_pc ret);

struct a_arg a_arg(struct a_string *name, struct a_type *type);

#endif
