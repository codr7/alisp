#ifndef ALISP_MULTI_H
#define ALISP_MULTI_H

#include "alisp/lset.h"
#include "alisp/types.h"

struct a_func;
struct a_string;

struct a_multi {
  struct a_string *name;
  uint8_t arg_count;
  struct a_lset funcs;
};

struct a_multi *a_multi(struct a_vm *vm, struct a_string *name, uint8_t arg_count);
struct a_multi *a_multi_init(struct a_multi *self, struct a_string *name, uint8_t arg_count);
bool a_multi_add(struct a_multi *self, struct a_func *func);
struct a_func *a_multi_specialize(struct a_multi *self, struct a_vm *vm, bool full);
a_pc_t a_multi_call(struct a_multi *self, struct a_vm *vm, enum a_call_flags flags, a_pc_t ret);

#endif
