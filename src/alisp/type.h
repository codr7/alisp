#ifndef ALISP_TYPE_H
#define ALISP_TYPE_H

#include <stdbool.h>
#include "alisp/types.h"

struct a_val;
struct a_vm;

struct a_type {
  struct a_vm *vm;
  void (*copy_val)(struct a_val *dst, struct a_val *src);
  bool (*deref_val)(struct a_val *val);
  a_refs refs;
};

struct a_type *a_type_init(struct a_type *self, struct a_vm *vm);
struct a_type *a_type_ref(struct a_type *self);
bool a_type_deref(struct a_type *self);

#endif
