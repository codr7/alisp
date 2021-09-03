#ifndef ALISP_LIB_H
#define ALISP_LIB_H

#include "alisp/lset.h"

struct a_string;
struct a_type;
struct a_vm;

struct a_lib {
  struct a_vm *vm;
  struct a_string *name;
  struct a_lset bindings;
};

struct a_lib *a_lib_init(struct a_lib *self, struct a_vm *vm, struct a_string *name);
void a_lib_deinit(struct a_lib *self);

struct a_val *a_lib_bind(struct a_lib *self, struct a_string *key, struct a_type *type);
struct a_func *a_lib_bind_func(struct a_lib *self, struct a_func *func);
struct a_prim *a_lib_bind_prim(struct a_lib *self, struct a_prim *prim);
struct a_type *a_lib_bind_type(struct a_lib *self, struct a_type *type);

struct a_val *a_lib_find(struct a_lib *self, const struct a_string *key);
bool a_lib_import(struct a_lib *self);

#endif
