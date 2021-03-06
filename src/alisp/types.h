#ifndef ALISP_TYPES_H
#define ALISP_TYPES_H

#include <stdint.h>

struct a_ls;
struct a_string;
struct a_type;
struct a_vm;

enum a_call_flags {A_CALL_CHECK = 1, A_CALL_DRETS = 2, A_CALL_MEM = 4, A_CALL_TCO = 8};

typedef long double a_float_t;
typedef int64_t a_int_t;
typedef uint16_t a_ref_count_t;
typedef int16_t a_reg_t;
typedef struct a_ls *a_pc_t;
typedef uint16_t a_type_id_t;

struct a_type *a_bool_type_init(struct a_type *self, struct a_vm *vm, struct a_string *name, struct a_type *super[]);
struct a_type *a_char_type_init(struct a_type *self, struct a_vm *vm, struct a_string *name, struct a_type *super[]);
struct a_type *a_fix_type_init(struct a_type *self, struct a_vm *vm, struct a_string *name, struct a_type *super[]);
struct a_type *a_float_type_init(struct a_type *self, struct a_vm *vm, struct a_string *name, struct a_type *super[]);
struct a_type *a_func_type_init(struct a_type *self, struct a_vm *vm, struct a_string *name, struct a_type *super[]);
struct a_type *a_int_type_init(struct a_type *self, struct a_vm *vm, struct a_string *name, struct a_type *super[]);
struct a_type *a_iter_type_init(struct a_type *self, struct a_vm *vm, struct a_string *name, struct a_type *super[]);
struct a_type *a_ls_type_init(struct a_type *self, struct a_vm *vm, struct a_string *name, struct a_type *super[]);
struct a_type *a_meta_type_init(struct a_type *self, struct a_vm *vm, struct a_string *name, struct a_type *super[]);
struct a_type *a_multi_type_init(struct a_type *self, struct a_vm *vm, struct a_string *name, struct a_type *super[]);
struct a_type *a_nil_type_init(struct a_type *self, struct a_vm *vm, struct a_string *name, struct a_type *super[]);
struct a_type *a_pair_type_init(struct a_type *self, struct a_vm *vm, struct a_string *name, struct a_type *super[]);
struct a_type *a_prim_type_init(struct a_type *self, struct a_vm *vm, struct a_string *name, struct a_type *super[]);
struct a_type *a_queue_type_init(struct a_type *self, struct a_vm *vm, struct a_string *name, struct a_type *super[]);
struct a_type *a_reg_type_init(struct a_type *self, struct a_vm *vm, struct a_string *name, struct a_type *super[]);
struct a_type *a_string_type_init(struct a_type *self, struct a_vm *vm, struct a_string *name, struct a_type *super[]);
struct a_type *a_sym_type_init(struct a_type *self, struct a_vm *vm, struct a_string *name, struct a_type *super[]);
struct a_type *a_thread_type_init(struct a_type *self, struct a_vm *vm, struct a_string *name, struct a_type *super[]);

#endif
