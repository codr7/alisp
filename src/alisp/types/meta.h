#ifndef ALISP_TYPES_META_H
#define ALISP_TYPES_META_H

struct a_string;
struct a_type;
struct a_vm;

struct a_type *a_meta_type_init(struct a_type *self, struct a_vm *vm, struct a_string *name);
  
#endif
