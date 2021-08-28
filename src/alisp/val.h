#ifndef ALISP_VAL_H
#define ALISP_VAL_H

#include "alisp/ls.h"
#include "alisp/types.h"

struct a_func;
struct a_prim;
struct a_type;
struct a_vm;

struct a_val {
  struct a_ls ls;
  struct a_type *type;

  union  {
    bool as_bool;
    struct a_func *as_func;
    int as_int;
    struct a_ls *as_ls;
    struct a_type *as_meta;
    struct a_prim *as_prim;
    a_reg as_reg;
  };
};

struct a_val *a_val(struct a_vm *vm, struct a_type *type);
struct a_val *a_val_init(struct a_val *self, struct a_type *type);
bool a_val_deref(struct a_val *self);

a_pc a_call(struct a_val *self, a_pc ret, bool check);
enum a_order a_compare(struct a_val *self, struct a_val *other);
void a_copy(struct a_val *self, struct a_val *source);
void a_dump(struct a_val *self);
bool a_equals(struct a_val *self, struct a_val *other);
bool a_is(struct a_val *self, struct a_val *other);
bool a_true(struct a_val *self);

#endif
