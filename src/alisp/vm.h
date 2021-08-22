#ifndef ALISP_VM_H
#define ALISP_VM_H

#include "alisp/ls.h"
#include "alisp/pool.h"
#include "alisp/scope.h"
#include "alisp/types.h"

#define A_PAGE_SIZE 32000
#define A_SCOPE_PAGE_SIZE 32

struct a_vm {
  struct a_pool pool;
  struct a_pool scope_pool;

  struct a_scope main;
  struct a_ls scopes;
  a_refs_t refs;
};

struct a_vm *a_vm_init(struct a_vm *self);
struct a_vm *a_vm_ref(struct a_vm *self);
bool a_vm_deref(struct a_vm *self);

#endif
