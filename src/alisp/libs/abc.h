#ifndef ALISP_LIBS_ABC_H
#define ALISP_LIBS_ABC_H

#include "alisp/lib.h"
#include "alisp/type.h"

struct a_abc_lib {
  struct a_lib lib;
  struct a_type any_type, bool_type, func_type, ls_type, int_type, meta_type, pair_type, prim_type, reg_type;
};

struct a_abc_lib *a_abc_lib_init(struct a_abc_lib *self, struct a_vm *vm);

#endif
