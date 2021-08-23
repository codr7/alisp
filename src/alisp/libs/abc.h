#ifndef ALISP_LIBS_ABC_H
#define ALISP_LIBS_ABC_H

#include "alisp/type.h"

struct a_abc_lib {
  struct a_type int_type;
  struct a_type register_type;
};

struct a_abc_lib *a_abc_lib_init(struct a_abc_lib *self, struct a_vm *vm);

#endif
