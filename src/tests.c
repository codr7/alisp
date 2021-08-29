#include <assert.h>
#include <inttypes.h>
#include <stdio.h>
#include "alisp/func.h"
#include "alisp/stack.h"
#include "alisp/string.h"
#include "alisp/type.h"
#include "alisp/vm.h"

static void test_push() {
  struct a_vm vm;
  a_vm_init(&vm);
  a_pc_t pc = a_pc(&vm);
  a_val_init(&a_emit(&vm, A_PUSH_OP)->as_push.val, &vm.abc.int_type)->as_int = 42;
  a_emit(&vm, A_STOP_OP);
  a_eval(&vm, pc);
  struct a_val *v = a_pop(&vm);
  assert(v->type == &vm.abc.int_type);
  assert(v->as_int == 42);
  a_val_deref(v);
  a_free(&vm.val_pool, v);
  a_vm_deinit(&vm);
}

static void test_bind() {
  struct a_vm vm;
  a_vm_init(&vm);
  a_pc_t pc = a_pc(&vm);
  a_reg_t reg = a_bind_reg(&vm, a_string(&vm, "foo"));
  a_val_init(&a_emit(&vm, A_PUSH_OP)->as_push.val, &vm.abc.int_type)->as_int = 42;
  a_emit(&vm, A_STORE_OP)->as_store.reg = reg;
  a_emit(&vm, A_LOAD_OP)->as_load.reg = reg;
  a_emit(&vm, A_STOP_OP);
  a_eval(&vm, pc);
  struct a_val *v = a_pop(&vm);
  assert(v->type == &vm.abc.int_type);
  assert(v->as_int == 42);
  a_val_deref(v);
  a_free(&vm.val_pool, v);
  a_vm_deinit(&vm);
}

static a_pc_t test_func_foo_body(struct a_func *self, struct a_vm *vm, a_pc_t ret) {
  struct a_val *y = a_pop(vm), *x = a_peek(vm);
  x->as_int += y->as_int;
  a_val_deref(y);
  a_free(&vm->val_pool, y);
  return ret;
}

static void test_func() {
  struct a_vm vm;
  a_vm_init(&vm);
  a_pc_t pc = a_pc(&vm);

  struct a_func f;
  
  a_func_init(&f, a_string(&vm, "foo"),
	      A_ARG(&vm,
		    {a_string(&vm, "x"), &vm.abc.int_type},
		    {a_string(&vm, "y"), &vm.abc.int_type}),
	      A_RET(&vm, &vm.abc.int_type));

  f.body = test_func_foo_body;
  struct a_val *t = a_val_init(a_malloc(&vm.val_pool, sizeof(struct a_val)), &vm.abc.func_type);
  t->as_func = a_func_ref(&f);
  a_emit(&vm, A_CALL_OP)->as_call.target = t;
  a_emit(&vm, A_STOP_OP);

  a_push(&vm, &vm.abc.int_type)->as_int = 35;
  a_push(&vm, &vm.abc.int_type)->as_int = 7;
  a_eval(&vm, pc);
  
  struct a_val *v = a_pop(&vm);
  assert(v);
  assert(v->type == &vm.abc.int_type);
  assert(v->as_int == 42);
  a_val_deref(v);
  a_free(&vm.val_pool, v);
  a_func_deref(&f, &vm);
  a_vm_deinit(&vm);
}

static void test_func_emit() {
  struct a_vm vm;
  a_vm_init(&vm);
  a_pc_t pc = a_pc(&vm);

  struct a_func f;
  
  a_func_init(&f, a_string(&vm, "foo"),
	      A_ARG(&vm),
	      A_RET(&vm, &vm.abc.int_type));

  a_func_begin(&f, &vm);
  a_val_init(&a_emit(&vm, A_PUSH_OP)->as_push.val, &vm.abc.int_type)->as_int = 42;
  a_func_end(&f, &vm);
  
  struct a_val *t = a_val_init(a_malloc(&vm.val_pool, sizeof(struct a_val)), &vm.abc.func_type);
  t->as_func = a_func_ref(&f);
  a_emit(&vm, A_CALL_OP)->as_call.target = t;
  a_emit(&vm, A_STOP_OP);
  a_eval(&vm, pc);
  
  struct a_val *v = a_pop(&vm);
  assert(v);
  assert(v->type == &vm.abc.int_type);
  assert(v->as_int == 42);
  a_val_deref(v);
  a_free(&vm.val_pool, v);
  a_func_deref(&f, &vm);
  a_vm_deinit(&vm);
}

int main() {
  test_push();
  test_bind();
  test_func();
  test_func_emit();
  return 0;
}
