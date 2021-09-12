#ifndef ALISP_LIBS_ABC_H
#define ALISP_LIBS_ABC_H

#include "alisp/lib.h"
#include "alisp/type.h"

struct a_abc_lib {
  struct a_lib lib;

  struct a_type any_type,
    bool_type, char_type, func_type,
    int_type, iter_type,
    list_type,
    meta_type, multi_type,
    nil_type, num_type,
    pair_type, prim_type,
    reg_type,
    seq_type, string_type,
    target_type, thread_type,
    undef_type;
};

struct a_abc_lib *a_abc_lib_init(struct a_abc_lib *self, struct a_vm *vm);

#endif
