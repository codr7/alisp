#include <stdio.h>
#include "alisp/fail.h"
#include "alisp/ls.h"
#include "alisp/stack.h"
#include "alisp/string.h"
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

void a_stack_type_dump(struct a_ls *stack) {
  putc('[', stdout);

  a_ls_do(stack, ls) {
    if (ls != stack->next) { putc(' ', stdout); }
    printf("%s", a_baseof(ls, struct a_val, ls)->type->name->data);
  }
  
  putc(']', stdout);
}

bool a_drop(struct a_vm *self, int count) {
  struct a_ls *vls = self->stack.prev;				   
  
  for (int i = count; i > 0; vls = vls->prev, i--) {
    if (vls == &self->stack) {
      a_fail("Not enough values on stack: %d", i);
      return false;
    }
    
    a_ls_pop(vls);
    struct a_val *v = a_baseof(vls, struct a_val, ls);
    a_val_deref(v);
    a_val_free(v, self);
  }

  return true;
}

struct a_val *a_peek(struct a_vm *self, int offset) {
  struct a_ls *ls = self->stack.prev;
  while (ls != &self->stack && offset--) { ls = ls->prev; }
  return (ls == &self->stack) ? NULL : a_baseof(ls, struct a_val, ls);
}

struct a_val *a_pop(struct a_vm *self) {
  return a_ls_null(&self->stack) ? NULL : a_baseof(a_ls_pop(self->stack.prev), struct a_val, ls);
}

struct a_val *a_push(struct a_vm *self, struct a_type *type) {
  struct a_val *v = a_val(type);
  a_ls_push(&self->stack, &v->ls);
  return v;
}

void a_reset(struct a_vm *self) {
  a_ls_do(&self->stack, ls) {
    struct a_val *v = a_baseof(ls, struct a_val, ls);
    a_val_deref(v);
    a_val_free(v, self);
  }

  a_ls_init(&self->stack);
}
