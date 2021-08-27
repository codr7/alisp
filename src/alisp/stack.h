#ifndef ALISP_STACK_H
#define ALISP_STACK_H

struct a_ls;
struct a_type;
struct a_vm;
struct a_val;

void a_stack_dump(struct a_ls *stack);

struct a_val *a_push(struct a_vm *self, struct a_type *type);
struct a_val *a_peek(struct a_vm *self);
struct a_val *a_pop(struct a_vm *self);

#endif
