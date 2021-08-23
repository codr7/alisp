#ifndef ALISP_STRING_H
#define ALISP_STRING_H

#include <stdbool.h>
#include <stdint.h>
#include "alisp/types.h"

struct a_vm;

typedef uint32_t a_string_length;

struct a_string {
  struct a_vm *vm;
  a_string_length length;
  a_ref_count ref_count;
  char data[];
};

struct a_string *a_string(struct a_vm *vm, const char *data);
struct a_string *a_string_ref(struct a_string *self);
bool a_string_deref(struct a_string *self);
enum a_order a_string_compare(const struct a_string *self, const struct a_string *other);

#endif
