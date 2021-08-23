#ifndef ALISP_VAL_H
#define ALISP_VAL_H

#include "alisp/ls.h"
#include "alisp/types.h"

struct a_type;

struct a_val {
  struct a_ls ls;
  struct a_type *type;

  union  {
    bool as_bool;
    int as_int;
    a_register as_register;
  };
};

struct a_val *a_val_init(struct a_val *self, struct a_type *type);
bool a_val_deref(struct a_val *self);
void a_val_copy(struct a_val *self, struct a_val *source);

#endif
