#ifndef ALISP_FUNC_H
#define ALISP_FUNC_H

#include <stdbool.h>
#include <string.h>
#include "alisp/types.h"

#define A_ARG(vm, ...)							\
  ({									\
    struct a_arg items[] = {__VA_ARGS__};				\
    struct a_args *args =						\
      a_malloc(&(vm)->pool, sizeof(struct a_args) + sizeof(items));	\
    args->count = sizeof(items) / sizeof(struct a_arg);			\
    memcpy(args->items, items, sizeof(items));				\
    args;								\
  })

#define A_RET(vm, ...)							\
  ({									\
    struct a_type *items[] = {__VA_ARGS__};				\
    struct a_rets *rets =						\
      a_malloc(&(vm)->pool, sizeof(struct a_rets) + sizeof(items));	\
    rets->count = sizeof(items) / sizeof(struct a_type *);		\
    memcpy(rets->items, items, sizeof(items));				\
    rets;								\
  })

struct a_vm;

struct a_arg {
  struct a_string *name;
  struct a_type *type;
};

struct a_args {
  uint8_t count;
  struct a_arg items[];
};

struct a_rets {
  uint8_t count;
  struct a_type *items[];
};

struct a_func {
  struct a_string *name;
  struct a_args *args;
  struct a_rets *rets;
  a_pc start_pc;
  struct a_scope *scope;
  a_pc (*body)(struct a_func *self, struct a_vm *vm, a_pc ret);
  a_ref_count ref_count;
};

struct a_func *a_func(struct a_vm *vm,
		      struct a_string *name,
		      struct a_args *args,
		      struct a_rets *rets);

struct a_func *a_func_init(struct a_func *self,
			   struct a_string *name,
			   struct a_args *args,
			   struct a_rets *rets);

struct a_func *a_func_ref(struct a_func *self);
bool a_func_deref(struct a_func *self, struct a_vm *vm);

void a_func_begin(struct a_func *self, struct a_vm *vm);
void a_func_end(struct a_func *self, struct a_vm *vm);

bool a_func_applicable(struct a_func *self, struct a_vm *vm);
a_pc a_func_call(struct a_func *self, struct a_vm *vm, enum a_call_flags flags, a_pc ret);

#endif
