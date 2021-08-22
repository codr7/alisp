#ifndef ALISP_VAL_H
#define ALISP_VAL_H

#include "alisp/ls.h"

struct a_type;

struct a_val {
  struct a_ls vm_stack;
  struct a_type *type;

  union  {
    int as_int;
  };
};

struct a_val *a_val_init(struct a_val *self, struct a_type *type);
void a_val_deinit(struct a_val *self);

#endif
