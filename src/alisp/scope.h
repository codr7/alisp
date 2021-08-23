#ifndef ALISP_SCOPE_H
#define ALISP_SCOPE_H

#include <inttypes.h>
#include <stdbool.h>
#include "alisp/lset.h"
#include "alisp/types.h"
#include "alisp/val.h"

struct a_scope {
  struct a_vm *vm;
  struct a_scope *outer;
  struct a_ls ls;
  struct a_lset bindings;
  a_register next_register;
  a_refs refs;
};

struct a_binding {
  struct a_ls set_items;
  struct a_string *key;
  struct a_val val;
};

struct a_scope *a_scope_init(struct a_scope *self, struct a_vm *vm, struct a_scope *outer);
struct a_scope *a_scope_ref(struct a_scope *self);
bool a_scope_deref(struct a_scope *self);

struct a_val *a_scope_bind(struct a_scope *self, struct a_string *key, struct a_type *type);

#endif
