#include <assert.h>
#include <string.h>
#include <stdio.h>
#include "alisp/string.h"
#include "alisp/vm.h"

struct a_string *a_string(struct a_vm *vm, const char *data) {
  a_string_length length = strlen(data);
  struct a_string *s = a_malloc(vm, sizeof(struct a_string)+length+1);
  s->vm = vm;
  s->length = length;
  strncpy(s->data, data, length);
  s->data[length] = 0;
  return s;
}

enum a_order a_string_compare(const struct a_string *self, const struct a_string *other) {
  int ord = strcmp(self->data, other->data);
  if (ord < 0) { return A_LT; }
  return ord ? A_GT : A_EQ;
}
