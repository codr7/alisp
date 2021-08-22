#include <assert.h>
#include <string.h>
#include "alisp/pool.h"
#include "alisp/string.h"
#include "alisp/vm.h"

struct a_string *a_string(struct a_vm *vm, const char *data) {
  a_string_length length = strlen(data);
  struct a_string *s = a_pool_malloc(&vm->string_pool, sizeof(struct a_string)+length+1);
  s->vm = vm;

  s->length = length;
  strncpy(s->data, data, length);
  s->data[length] = 0;

  s->refs = 1;
  return s;
}

struct a_string *a_string_ref(struct a_string *self) {
  self->refs++;
  return self;
}

bool a_string_deref(struct a_string *self) {
  assert(self->refs);
  if (--self->refs) { return false; }
  a_pool_free(&self->vm->string_pool, self);
  return true;
}

enum a_order a_string_compare(struct a_string *self, struct a_string *other) {
  return strcmp(self->data, other->data);
}
