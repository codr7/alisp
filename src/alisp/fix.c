#include <assert.h>
#include <inttypes.h>
#include <stdio.h>
#include "alisp/fix.h"
#include "alisp/limits.h"
#include "alisp/macros.h"

static int64_t POW[A_MAX_FIX_SCALE] = {
  1, 10, 100, 1000, 10000, 100000, 1000000, 10000000
};

int64_t a_pow(uint8_t scale) {    
  assert(scale < A_MAX_FIX_SCALE);
  return POW[scale];
}

struct a_fix a_fix_new(int64_t val, uint8_t scale) {
  return (struct a_fix) {.val = val, .scale = scale};
}

int64_t a_fix_trunc(struct a_fix self) { return self.val / a_pow(self.scale); }

int64_t a_fix_frac(struct a_fix self) { return self.val % a_pow(self.scale); }

void a_fix_dump(struct a_fix self) {
  int64_t t = a_fix_trunc(self);  
  printf("%" PRId64, t);
  putc('.', stdout);
  int64_t f = a_fix_frac(self);
  printf("%0*" PRId64, self.scale, (t < 0) ? -f : f);
}

struct a_fix a_fix_add(struct a_fix left, struct a_fix right) {
  uint8_t ls = left.scale, rs = right.scale, s = a_max(ls, rs);
  int64_t lv = left.val, rv = right.val;
  return a_fix_new(((ls == s) ? lv : lv*a_pow(s)/a_pow(ls)) + ((rs == s) ? rv : rv*a_pow(s)/a_pow(rs)), s);
}

struct a_fix a_fix_sub(struct a_fix left, struct a_fix right) {
  uint8_t ls = left.scale, rs = right.scale, s = a_max(ls, rs);
  int64_t lv = left.val, rv = right.val;
  return a_fix_new(((ls == s) ? lv : lv*a_pow(s)/a_pow(ls)) - ((rs == s) ? rv : rv*a_pow(s)/a_pow(rs)), s);
}

enum a_order a_fix_compare(struct a_fix left, struct a_fix right) {
  int64_t lv = left.val, rv = right.val;
  uint8_t ls = left.scale, rs = right.scale;
  if (rs != ls) { rv *= a_pow(rs)/a_pow(ls); }
  if (lv < rv) { return A_LT; }
  return (lv == rv) ? A_EQ : A_GT;
}
