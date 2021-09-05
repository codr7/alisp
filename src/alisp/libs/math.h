#ifndef ALISP_LIBS_MATH_H
#define ALISP_LIBS_MATH_H

#include "alisp/lib.h"
#include "alisp/type.h"

struct a_math_lib {
  struct a_lib lib;
  struct a_type fix_type;
};

struct a_math_lib *a_math_lib_init(struct a_math_lib *self, struct a_vm *vm);

#endif
