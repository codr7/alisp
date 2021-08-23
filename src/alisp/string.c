#include <assert.h>
#include <string.h>
#include "alisp/pool.h"
#include "alisp/string.h"
#include "alisp/vm.h"

struct a_string *a_string(struct a_vm *vm, const char *data) {
  a_string_length length = strlen(data);
  struct a_string *s = a_malloc(&vm->string_pool, sizeof(struct a_string)+length+1);
  s->vm = vm;

  s->length = length;
  strncpy(s->data, data, length);
  s->data[length] = 0;

  s->ref_count = 0;
  return s;
}

struct a_string *a_string_ref(struct a_string *self) {
  self->ref_count++;
  return self;
}

bool a_string_deref(struct a_string *self) {
  assert(self->ref_count);
  if (--self->ref_count) { return false; }
  a_free(&self->vm->string_pool, self);
  return true;
}

enum a_order a_string_compare(const struct a_string *self, const struct a_string *other) {
  return strcmp(self->data, other->data);
}
