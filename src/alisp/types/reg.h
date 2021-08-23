#ifndef ALISP_TYPES_REG_H
#define ALISP_TYPES_REG_H

struct a_string;
struct a_type;
struct a_vm;

struct a_type *a_reg_type_init(struct a_type *self, struct a_vm *vm, struct a_string *name);
  
#endif
