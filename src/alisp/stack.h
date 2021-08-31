#ifndef ALISP_STACK_H
#define ALISP_STACK_H

#include <stdbool.h>

struct a_ls;
struct a_type;
struct a_vm;
struct a_val;

void a_stack_dump(struct a_ls *stack);
void a_stack_type_dump(struct a_ls *stack);

bool a_drop(struct a_vm *self, int count);
struct a_val *a_peek(struct a_vm *self, int offset);
struct a_val *a_pop(struct a_vm *self);
struct a_val *a_push(struct a_vm *self, struct a_type *type);
void a_reset(struct a_vm *self);

#endif
