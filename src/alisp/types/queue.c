#include <stdio.h>
#include "alisp/queue.h"
#include "alisp/string.h"
#include "alisp/type.h"
#include "alisp/val.h"

static void copy_val(struct a_val *dst, struct a_val *src) { dst->as_queue = a_queue_ref(src->as_queue); }

static bool deref_val(struct a_val *val) { return a_queue_deref(val->as_queue); }

static void dump_val(struct a_val *val) { printf("Queue(%p)", val->as_queue); }

struct a_type *a_queue_type_init(struct a_type *self,
				struct a_vm *vm,
				struct a_string *name,
				struct a_type *super[]) {
  a_type_init(self, vm, name, super);
  self->copy_val = copy_val;
  self->deref_val = deref_val;
  self->dump_val = dump_val;
  return self;
}
