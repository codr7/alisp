#include <assert.h>
#include <string.h>
#include <stdio.h>
#include "alisp/string.h"
#include "alisp/vm.h"

struct a_string *a_string(struct a_vm *vm, const char *data) {
  struct a_ls *found = a_lset_find(&vm->strings, data);
  if (found) { return a_baseof(found, struct a_string, ls); }
  a_string_length length = strlen(data);
  struct a_string *s = a_malloc(vm, sizeof(struct a_string)+length+1);
  s->length = length;
  strncpy(s->data, data, length);
  s->data[length] = 0;
  a_lset_add(&vm->strings, &s->ls, false);
  return s;
}

void a_string_free(struct a_string *self, struct a_vm *vm) { a_free(vm, self); }

enum a_order a_string_compare(const struct a_string *left, const struct a_string *right) {
  if (left == right) { return A_EQ; }  
  return (strcmp(left->data, right->data) < 0) ? A_LT : A_GT;
}

