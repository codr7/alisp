#ifndef ALISP_TYPE_H
#define ALISP_TYPE_H

#include <stdbool.h>
#include "alisp/compare.h"
#include "alisp/types.h"

#define A_MAX_TYPE_ID 32

#define A_SUPER(...)				\
  (struct a_type *[]){__VA_ARGS__}

struct a_string;
struct a_val;
struct a_vm;

typedef uint16_t a_type_id;

struct a_type {
  struct a_vm *vm;
  a_type_id id;
  struct a_string *name;
  struct a_type *super[A_MAX_TYPE_ID]; 
  a_pc_t (*call_val)(struct a_val *val, enum a_call_flags flags, a_pc_t ret);
  enum a_order (*compare_val)(struct a_val *x, struct a_val *y);
  void (*copy_val)(struct a_val *dst, struct a_val *src);
  bool (*deref_val)(struct a_val *val);
  void (*dump_val)(struct a_val *val);
  bool (*equals_val)(struct a_val *x, struct a_val *y);
  bool (*is_val)(struct a_val *x, struct a_val *y);
  bool (*true_val)(struct a_val *val);
};

struct a_type *a_type_init(struct a_type *self, struct a_vm *vm, struct a_string *name, struct a_type *super[]);
bool a_isa(struct a_type *self, struct a_type *super);

#endif
