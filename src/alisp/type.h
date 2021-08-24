#ifndef ALISP_TYPE_H
#define ALISP_TYPE_H

#include <stdbool.h>
#include "alisp/types.h"

#define A_MAX_TYPE_ID 32

struct a_string;
struct a_val;
struct a_vm;

typedef uint64_t a_type_id;

struct a_type {
  struct a_vm *vm;
  a_type_id id;
  struct a_string *name;
  struct a_type *super_types[A_MAX_TYPE_ID]; 
  a_pc (*call_val)(struct a_val *val, a_pc ret, bool check);
  void (*copy_val)(struct a_val *dst, struct a_val *src);
  bool (*deref_val)(struct a_val *val);
  a_ref_count ref_count;
};

struct a_type *a_type_init(struct a_type *self, struct a_vm *vm, struct a_string *name, struct a_type *super_types[]);
struct a_type *a_type_ref(struct a_type *self);
bool a_type_deref(struct a_type *self);

bool a_isa(struct a_type *self, struct a_type *super);

#endif
