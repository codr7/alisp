#ifndef ALISP_TYPES_INT_H
#define ALISP_TYPES_INT_H

struct a_string;
struct a_type;
struct a_vm;

struct a_type *a_int_type_init(struct a_type *self, struct a_vm *vm, struct a_string *name);
  
#endif
