#ifndef ALISP_TYPES_REGISTER_H
#define ALISP_TYPES_REGISTER_H

struct a_string;
struct a_type;
struct a_vm;

struct a_type *a_register_type_init(struct a_type *self, struct a_vm *vm, struct a_string *name);
  
#endif
