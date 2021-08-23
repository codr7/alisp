#include <assert.h>
#include <stdio.h>
#include "alisp/string.h"
#include "alisp/type.h"
#include "alisp/types/int.h"
#include "alisp/vm.h"

void test_push() {
  struct a_vm vm;
  a_vm_init(&vm);
  a_pc pc = a_next_pc(&vm);
  a_val_init(&a_emit(&vm, A_PUSH)->as_push.val, &vm.abc.int_type)->as_int = 42;
  a_emit(&vm, A_STOP);
  a_eval(&vm, pc);
  struct a_val *v = a_pop(&vm);
  assert(v->type == &vm.abc.int_type);
  assert(v->as_int == 42);
  a_val_deref(v);
  a_free(&vm.val_pool, v);
  a_vm_deref(&vm);
}

int main() {
  test_push();
  return 0;
}
