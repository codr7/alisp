#ifndef ALISP_STRING_H
#define ALISP_STRING_H

#include <stdint.h>
#include "alisp/compare.h"
#include "alisp/ls.h"

struct a_vm;

typedef uint32_t a_string_length;

struct a_string {
  struct a_ls ls;
  a_string_length length;
  char data[];
};

struct a_string *a_string(struct a_vm *vm, const char *data);
void a_string_free(struct a_string *self, struct a_vm *vm);

enum a_order a_string_compare(const struct a_string *left, const struct a_string *right);

#endif
