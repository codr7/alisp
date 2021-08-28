#ifndef ALISP_PARSERS_H
#define ALISP_PARSERS_H

struct a_form;
struct a_parser;

struct a_form *a_skip_space(struct a_parser *self);
struct a_form *a_parse_id(struct a_parser *self);
struct a_form *a_parse_int(struct a_parser *self);

#endif