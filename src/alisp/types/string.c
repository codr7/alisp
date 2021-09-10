#include <stdio.h>
#include "alisp/iter.h"
#include "alisp/string.h"
#include "alisp/type.h"
#include "alisp/val.h"
#include "alisp/vm.h"

static enum a_order compare_val(struct a_val *x, struct a_val *y) {
  return a_string_compare(x->as_string, y->as_string);
}

static void copy_val(struct a_val *dst, struct a_val *src) { dst->as_string = src->as_string; }

static void dump_val(struct a_val *val) { printf("\"%s\"", val->as_string->data); }

static bool is_val(struct a_val *x, struct a_val *y) { return x->as_string == y->as_string; }

static struct a_val *iter_body(struct a_iter *self, struct a_vm *vm) {
  struct a_val
    *s = a_baseof(self->data.next, struct a_val, ls),
    *i = a_baseof(self->data.next->next, struct a_val, ls);

  if (i->as_int == s->as_string->length) { return NULL; }
  struct a_val *out = a_val_new(&vm->abc.char_type);
  out->as_char = s->as_string->data[i->as_int++];
  return out;
}

static struct a_iter *iter_val(struct a_val *val) {
  struct a_vm *vm = val->type->vm;
  struct a_iter *it = a_iter_new(vm, iter_body);
  struct a_val *s = a_val_new(&vm->abc.string_type), *i = a_val_new(&vm->abc.int_type);
  s->as_string = val->as_string;
  i->as_int = 0;
  a_ls_push(&it->data, &s->ls);
  a_ls_push(&it->data, &i->ls);
  return it;
}

static bool true_val(struct a_val *val) { return val->as_string->length; }

struct a_type *a_string_type_init(struct a_type *self, struct a_vm *vm, struct a_string *name, struct a_type *super[]) {
  a_type_init(self, vm, name, super);
  self->compare_val = compare_val;
  self->copy_val = copy_val;
  self->dump_val = dump_val;
  self->is_val = is_val;
  self->iter_val = iter_val;
  self->true_val = true_val;
  return self;
}
