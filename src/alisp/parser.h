#ifndef ALISP_PARSER_H
#define ALISP_PARSER_H

#include "alisp/form.h"
#include "alisp/ls.h"
#include "alisp/pos.h"
#include "alisp/stream.h"

struct a_parser;
struct a_vm;

typedef struct a_form *(*a_parser_body)(struct a_parser *);

struct a_form_parser {
  struct a_ls ls;
  a_parser_body body;
};
  
struct a_parser {
  struct a_vm *vm;
  struct a_pos pos;
  struct a_stream in;
  struct a_ls forms;
  struct a_ls prefix, suffix;
};

struct a_parser *a_parser_init(struct a_parser *self, struct a_vm *vm, struct a_string *source);
void a_parser_deinit(struct a_parser *self);
struct a_form_parser *a_parser_add_prefix(struct a_parser *self, a_parser_body body);
struct a_form_parser *a_parser_add_suffix(struct a_parser *self, a_parser_body body);
struct a_form *a_parser_push(struct a_parser *self, enum a_form_type type, struct a_pos pos);
struct a_form *a_parser_pop(struct a_parser *self);
struct a_form *a_parser_pop_last(struct a_parser *self);
bool a_parser_next(struct a_parser *self);
struct a_form *a_parser_peek_next(struct a_parser *self);
struct a_form *a_parser_pop_next(struct a_parser *self);

#endif
