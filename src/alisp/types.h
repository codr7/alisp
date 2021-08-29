#ifndef ALISP_TYPES_H
#define ALISP_TYPES_H

#include <stdint.h>

#define A_REG_COUNT 64

struct a_ls;
struct a_string;
struct a_type;
struct a_vm;

enum a_call_flags {A_CALL_DRETS = 1, A_CALL_CHECK = 3};

typedef uint16_t a_ref_count;
typedef uint16_t a_reg;
typedef struct a_ls *a_pc;

struct a_type *a_bool_type_init(struct a_type *self, struct a_vm *vm, struct a_string *name, struct a_type *super[]);

struct a_type *a_func_type_init(struct a_type *self,
				struct a_vm *vm,
				struct a_string *name,
				struct a_type *super[]);

struct a_type *a_int_type_init(struct a_type *self, struct a_vm *vm, struct a_string *name, struct a_type *super[]);

struct a_type *a_ls_type_init(struct a_type *self, struct a_vm *vm, struct a_string *name, struct a_type *super[]);

struct a_type *a_meta_type_init(struct a_type *self, struct a_vm *vm, struct a_string *name, struct a_type *super[]);

struct a_type *a_pair_type_init(struct a_type *self,
				struct a_vm *vm,
				struct a_string *name,
				struct a_type *super[]);

struct a_type *a_prim_type_init(struct a_type *self,
				struct a_vm *vm,
				struct a_string *name,
				struct a_type *super[]);

struct a_type *a_reg_type_init(struct a_type *self, struct a_vm *vm, struct a_string *name, struct a_type *super[]);

#endif
