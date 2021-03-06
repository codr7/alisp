#ifndef ALISP_VAL_H
#define ALISP_VAL_H

#include "alisp/fix.h"
#include "alisp/ls.h"
#include "alisp/pair.h"
#include "alisp/types.h"

struct a_func;
struct a_iter;
struct a_multi;
struct a_prim;
struct a_queue;
struct a_string;
struct a_thread;
struct a_type;
struct a_vm;

struct a_val {
  struct a_ls ls;
  struct a_type *type;
  bool undef;
  
  union  {
    bool as_bool;
    char as_char;
    struct a_fix as_fix;
    a_float_t as_float;
    struct a_func *as_func;
    a_int_t as_int;
    struct a_iter *as_iter;
    struct a_ls *as_ls;
    struct a_type *as_meta;
    struct a_multi *as_multi;
    struct a_pair as_pair;
    struct a_prim *as_prim;
    struct a_queue *as_queue;
    a_reg_t as_reg;
    struct a_string *as_string;
    struct a_string *as_sym;
    struct a_thread *as_thread;
  };
};

struct a_val *a_val_new(struct a_type *type);
struct a_val *a_val_init(struct a_val *self, struct a_type *type);
void a_val_free(struct a_val *self, struct a_vm *vm);

a_pc_t a_call(struct a_val *self, enum a_call_flags flags, a_pc_t ret);
struct a_val *a_clone(struct a_val *self, struct a_val *source);
enum a_order a_compare(struct a_val *self, struct a_val *other);
struct a_val *a_copy(struct a_val *self, struct a_val *source);
bool a_deref(struct a_val *self);
void a_dump(struct a_val *self);
bool a_equals(struct a_val *self, struct a_val *other);
bool a_is(struct a_val *self, struct a_val *other);
struct a_iter *a_iter(struct a_val *self);
bool a_true(struct a_val *self);

#endif
