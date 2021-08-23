#ifndef ALISP_BINDING_H
#define ALISP_BINDING_H

#include "alisp/compare.h"
#include "alisp/val.h"

struct a_ls;
struct a_string;

struct a_binding {
  struct a_string *key;
  struct a_val val;
};

void a_binding_deinit(struct a_binding *self);
const void *a_binding_key(const struct a_ls *self);
enum a_order a_binding_compare(const void *x, const void *y);

#endif
