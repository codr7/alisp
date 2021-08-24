#ifndef ALISP_STRING_H
#define ALISP_STRING_H

#include <stdbool.h>
#include <stdint.h>

struct a_vm;

typedef uint32_t a_string_length;

struct a_string {
  struct a_vm *vm;
  a_string_length length;
  char data[];
};

struct a_string *a_string(struct a_vm *vm, const char *data);
enum a_order a_string_compare(const struct a_string *self, const struct a_string *other);

#endif
