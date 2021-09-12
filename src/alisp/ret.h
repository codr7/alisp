#ifndef ALISP_RET_H
#define ALISP_RET_H

#include <stdint.h>
#include "alisp/limits.h"

#define A_RET(vm, ...)							\
  ({									\
    struct a_type *items[] = {__VA_ARGS__};				\
    struct a_rets rets;							\
    rets.count = sizeof(items) / sizeof(struct a_type *);		\
    memcpy(rets.items, items, sizeof(items));				\
    rets;								\
  })

struct a_type;

struct a_rets {
  uint8_t count;
  struct a_type *items[A_MAX_RETS];
};

#endif
