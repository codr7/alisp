#include <ctype.h>
#include "alisp/parser.h"
#include "alisp/parsers.h"
#include "alisp/string.h"
#include "alisp/vm.h"

struct a_form *a_skip_space(struct a_parser *self) {
  char c = 0;
  
  while ((c = a_stream_getc(&self->in))) {
    switch (c) {
    case ' ':
    case '\t':
      self->pos.column++;
      break;
    case '\n':
      self->pos.line++;
      self->pos.column = A_MIN_COLUMN;
      break;
    default:
      a_stream_ungetc(&self->in);
      return NULL;
    }
  }
  
  return NULL;
}

struct a_form *a_parse_id(struct a_parser *self) {
  struct a_pos fpos = self->pos;
  struct a_stream out;
  a_stream_init(&out);
  char c = 0;

  while ((c = a_stream_getc(&self->in))) {
    if (isspace(c) ||
	c == '(' || c == ')' || c == '[' || c == ']' || c == ':' || c == '.' || c == '\'' || c == ',') {
      a_stream_ungetc(&self->in);
      break;
    }

    a_stream_putc(&out, c);
    self->pos.column++;
  }

  if (!out.length) { return NULL; }

  if (out.length == 1 && out.data[0] == '_') {
    a_stream_deinit(&out);
    return a_parser_push(self, A_NOP_FORM, fpos);
  }
  
  struct a_form *f = a_parser_push(self, A_ID_FORM, fpos);
  f->as_id.name = a_string(self->vm, out.data);
  a_stream_deinit(&out);
  return f;
}

struct a_form *a_parse_int(struct a_parser *self) {
  int v = 0;
  bool neg = false;
  struct a_pos fpos = self->pos;
  
  char c = a_stream_getc(&self->in);
  if (!c) { return NULL; }

  if (c == '-') {
    c = a_stream_getc(&self->in);

    if (isdigit(c)) {
      neg = true;
      self->pos.column++;
    } else {
      a_stream_ungetc(&self->in);
    }

    a_stream_ungetc(&self->in);
  } else {
      a_stream_ungetc(&self->in);    
  }

   while ((c = a_stream_getc(&self->in))) {
     if (!isdigit(c)) {
       a_stream_ungetc(&self->in);
       break;
     }

     v *= 10;
     v += c - '0';
     self->pos.column++;
   }

   if (self->pos.column == fpos.column) { return NULL; }
   struct a_form *f = a_parser_push(self, A_LITERAL_FORM, fpos);
   a_val_init(&f->as_literal.val, &self->vm->abc.int_type)->as_int = v;
   return f;
}
