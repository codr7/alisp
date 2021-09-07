#ifndef ALISP_UTILS_H
#define ALISP_UTILS_H

struct a_string;
struct a_vm;

struct a_string *a_format(struct a_vm *vm, const char *spec, ...);

enum a_order a_strcmp(const char *left, const char *right);

#endif
