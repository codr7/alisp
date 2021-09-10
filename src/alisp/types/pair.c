#include "alisp/iter.h"
#include "alisp/stack.h"
#include "alisp/type.h"
#include "alisp/val.h"
#include "alisp/vm.h"

static enum a_order compare_val(struct a_val *x, struct a_val *y) {
  switch (a_compare(x->as_pair.left, y->as_pair.left)) {
  case A_LT:
    return A_LT;
  case A_GT:
    return A_GT;
  case A_EQ:
    break;
  }
  
  switch (a_compare(x->as_pair.right, y->as_pair.right)) {
  case A_LT:
    return A_LT;
  case A_GT:
    return A_GT;
  case A_EQ:
    break;
  }

  return A_EQ;
}

static void copy_val(struct a_val *dst, struct a_val *src) {
  struct a_pair *dp = &dst->as_pair, *sp = &src->as_pair;
  dp->left = a_copy(a_val_new(sp->left->type), sp->left);
  dp->right = a_copy(a_val_new(sp->right->type), sp->right);
}

static bool deref_val(struct a_val *val) {
  struct a_pair *self = &val->as_pair;
  bool l = a_deref(self->left), r = a_deref(self->right);
  if (l && r) { return true; }
  if (l) { self->left = a_val_new(&val->type->vm->abc.nil_type); }
  if (r) { self->right = a_val_new(&val->type->vm->abc.nil_type); }
  return false;
}

static void dump_val(struct a_val *val) { a_pair_dump(&val->as_pair); }

static bool equals_val(struct a_val *x, struct a_val *y) {
  return a_equals(x->as_pair.left, y->as_pair.left) && a_equals(x->as_pair.right, y->as_pair.right);
}

static bool is_val(struct a_val *x, struct a_val *y) {
  return a_is(x->as_pair.left, y->as_pair.left) && a_is(x->as_pair.right, y->as_pair.right);
}

static struct a_val *iter_body(struct a_iter *self, struct a_vm *vm) {
  if (a_ls_null(&self->data)) { return NULL; }
  struct a_val *i = a_baseof(a_ls_pop(self->data.next), struct a_val, ls);
  
  if (i->type == &vm->abc.pair_type) {
    a_ls_push(&self->data, &i->as_pair.right->ls);
    return i->as_pair.left;
  }

  return i;
}

static struct a_iter *iter_val(struct a_val *val) {
  struct a_vm *vm = val->type->vm;
  struct a_iter *it = a_iter_new(vm, iter_body);
  struct a_val *i = a_copy(a_val_new(&vm->abc.pair_type), val);
  a_ls_push(&it->data, &i->ls);
  return it;
}

static bool true_val(struct a_val *val) {
  return a_true(val->as_pair.left) && a_true(val->as_pair.right);
}

struct a_type *a_pair_type_init(struct a_type *self, struct a_vm *vm, struct a_string *name, struct a_type *super[]) {
  a_type_init(self, vm, name, super);
  self->compare_val = compare_val;
  self->copy_val = copy_val;
  self->deref_val = deref_val;
  self->dump_val = dump_val;
  self->equals_val = equals_val;
  self->is_val = is_val;
  self->iter_val = iter_val;
  self->true_val = true_val;
  return self;
}
