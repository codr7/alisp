#include <stdio.h>
#include "alisp/iter.h"
#include "alisp/queue.h"
#include "alisp/string.h"
#include "alisp/type.h"
#include "alisp/val.h"
#include "alisp/vm.h"

static void copy_val(struct a_val *dst, struct a_val *src) { dst->as_queue = a_queue_ref(src->as_queue); }

static bool deref_val(struct a_val *val) { return a_queue_deref(val->as_queue); }

static void dump_val(struct a_val *val) { printf("Queue(%p)", val->as_queue); }

static struct a_val *iter_body(struct a_iter *self, struct a_vm *vm) {
  struct a_queue *q = a_baseof(self->data.next, struct a_val, ls)->as_queue;
  return a_queue_pop(q);
}

static struct a_iter *iter_val(struct a_val *val) {
  struct a_vm *vm = val->type->vm;
  struct a_iter *it = a_iter_new(vm, iter_body);
  struct a_val *q = a_val_new(&vm->abc.queue_type);
  q->as_queue = a_queue_ref(val->as_queue);
  a_ls_push(&it->data, &q->ls);
  return it;
}

struct a_type *a_queue_type_init(struct a_type *self,
				struct a_vm *vm,
				struct a_string *name,
				struct a_type *super[]) {
  a_type_init(self, vm, name, super);
  self->copy_val = copy_val;
  self->deref_val = deref_val;
  self->dump_val = dump_val;
  self->iter_val = iter_val;
  return self;
}
