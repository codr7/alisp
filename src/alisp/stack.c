#include <stdio.h>
#include "alisp/ls.h"
#include "alisp/pool.h"
#include "alisp/stack.h"
#include "alisp/val.h"
#include "alisp/vm.h"

void a_stack_dump(struct a_ls *stack) {
  putc('[', stdout);

  a_ls_do(stack, ls) {
    if (ls != stack->next) { putc(' ', stdout); }
    a_dump(a_baseof(ls, struct a_val, ls));
  }
  
  putc(']', stdout);
}

struct a_val *a_push(struct a_vm *self, struct a_type *type) {
  struct a_val *v = a_val_init(a_malloc(&self->val_pool, sizeof(struct a_val)), type);
  a_ls_push(&self->stack, &v->ls);
  return v;
}

struct a_val *a_peek(struct a_vm *self) {
  return a_ls_null(&self->stack) ? NULL : a_baseof(self->stack.prev, struct a_val, ls);
}

struct a_val *a_pop(struct a_vm *self) {
  return a_ls_null(&self->stack) ? NULL : a_baseof(a_ls_pop(self->stack.prev), struct a_val, ls);
}
