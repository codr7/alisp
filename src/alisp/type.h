#ifndef ALISP_TYPE_H
#define ALISP_TYPE_H

#include <stdbool.h>
#include "alisp/compare.h"
#include "alisp/types.h"

#define A_MAX_TYPE_ID 256

#define _A_SUPER(...)				\
  (struct a_type *[]){__VA_ARGS__}

#define A_SUPER(...)				\
  _A_SUPER(__VA_ARGS__, NULL)

struct a_string;
struct a_val;
struct a_vm;

struct a_type {
  struct a_vm *vm;
  a_type_id_t id;
  struct a_string *name;
  struct a_type *super[A_MAX_TYPE_ID]; 
  a_pc_t (*call_val)(struct a_val *val, enum a_call_flags flags, a_pc_t ret);
  void (*clone_val)(struct a_val *dst, struct a_val *src);
  enum a_order (*compare_val)(struct a_val *x, struct a_val *y);
  void (*copy_val)(struct a_val *dst, struct a_val *src);
  bool (*deref_val)(struct a_val *val);
  void (*dump_val)(struct a_val *val);
  bool (*equals_val)(struct a_val *x, struct a_val *y);
  bool (*is_val)(struct a_val *x, struct a_val *y);
  struct a_iter *(*iter_val)(struct a_val *val);
  bool (*true_val)(struct a_val *val);
};

struct a_type *a_type_new(struct a_vm *vm, struct a_string *name, struct a_type *super[]);
struct a_type *a_type_init(struct a_type *self, struct a_vm *vm, struct a_string *name, struct a_type *super[]);
struct a_type *a_type_clone(struct a_type *self, struct a_vm *dst_vm);

bool a_isa(struct a_type *self, struct a_type *super);

#endif
