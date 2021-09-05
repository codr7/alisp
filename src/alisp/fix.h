#ifndef ALISP_FIX_H
#define ALISP_FIX_H

#include <stdbool.h>
#include <stdint.h>
#include "alisp/compare.h"

struct a_fix {
  int64_t val;
  uint8_t scale;
};

int64_t a_pow(uint8_t scale);

struct a_fix a_fix(int64_t val, uint8_t scale);
int64_t a_fix_trunc(struct a_fix self);
int64_t a_fix_frac(struct a_fix self);
void a_fix_dump(struct a_fix self);

struct a_fix a_fix_add(struct a_fix left, struct a_fix right);
struct a_fix a_fix_sub(struct a_fix left, struct a_fix right);
enum a_order a_fix_compare(struct a_fix left, struct a_fix right);

#endif
