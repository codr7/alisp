#ifndef ALISP_PRIM_H
#define ALISP_PRIM_H

#include <stdbool.h>
#include <string.h>
#include "alisp/types.h"

struct a_vm;

struct a_prim {
  struct a_string *name;
  uint8_t min_args, max_args;
  a_pc (*body)(struct a_prim *self, struct a_ls *args);
  a_ref_count ref_count;
};

struct a_prim *a_prim_init(struct a_prim *self,
			   struct a_string *name,
			   uint8_t min_args,
			   uint8_t max_args);

struct a_prim *a_prim_ref(struct a_prim *self);
bool a_prim_deref(struct a_prim *self);
bool a_prim_applicable(struct a_prim *self);
bool a_prim_call(struct a_prim *self, struct a_vm *vm, struct a_ls *args);

#endif
