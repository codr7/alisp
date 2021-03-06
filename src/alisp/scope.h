#ifndef ALISP_SCOPE_H
#define ALISP_SCOPE_H

#include <inttypes.h>
#include <stdbool.h>
#include "alisp/binding.h"
#include "alisp/lset.h"
#include "alisp/types.h"

struct a_string;

struct a_scope {
  struct a_vm *vm;
  struct a_scope *outer;
  struct a_ls ls;
  struct a_lset bindings;
  a_reg_t next_reg;
  a_ref_count_t ref_count;
};

struct a_scope *a_scope_init(struct a_scope *self, struct a_vm *vm, struct a_scope *outer);
struct a_scope *a_scope_ref(struct a_scope *self);
bool a_scope_deref(struct a_scope *self);

struct a_val *a_scope_bind(struct a_scope *self, struct a_string *key, struct a_type *type);
a_reg_t a_scope_bind_reg(struct a_scope *self, struct a_string *key);
bool a_scope_unbind(struct a_scope *self, struct a_string *key);
struct a_val *a_scope_find(struct a_scope *self, const struct a_string *key);

#endif
