#include <ctype.h>
#include "alisp/fail.h"
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

struct a_form *a_parse_call(struct a_parser *self) {  
  struct a_pos fpos = self->pos;
  if (!a_parser_check(self, '(')) { return NULL; }
  struct a_form *t = a_parser_pop_next(self);

  if (!t) {
    a_fail("Missing call target");
    return NULL;
  }

  struct a_form *cf = a_malloc(self->vm, sizeof(struct a_form));
  a_form_init(cf, A_CALL_FORM, fpos);
  cf->as_call.target = t;
  
  while (true) {
    a_skip_space(self);
    char c = a_stream_getc(&self->in);

    if (c == ')') {
      self->pos.column++;
      break;
    }

    if (c) { a_stream_ungetc(&self->in); }
    struct a_form *f = a_parser_pop_next(self);
    
    if (!f) {
      a_form_deref(cf, self->vm);
      a_fail("Open call form");
      return NULL;
    }

    a_ls_push(&cf->as_call.args, &f->ls);
  }

  a_ls_push(&self->forms, &cf->ls);
  return cf;
}

struct a_form *a_parse_dot(struct a_parser *self) {
  struct a_pos fpos = self->pos;
  if (!a_parser_check(self, '.')) { return NULL; }
  struct a_form *arg = a_parser_pop_last(self);

  if (!arg) {
    a_fail("Missing first argument");
    return NULL;
  }
  
  struct a_form *call = a_parser_peek_next(self);

  if (!call) {
    a_fail("Missing call form");
    return NULL;
  }

  if (call->type != A_CALL_FORM) {
    a_fail("Invalid call form: %d", call->type);
    return NULL;
  }

  call->pos = fpos;
  a_ls_push(call->as_call.args.next, &arg->ls);
  return call;
}

struct a_form *a_parse_fix(struct a_parser *self, int64_t trunc) {
  struct a_pos fpos = self->pos;
  if (!a_parser_check(self, '.')) { return NULL; }

  char c = a_stream_getc(&self->in);
  if (!c) { return false; }
  a_stream_ungetc(&self->in);
  
  if (!isdigit(c)) {
      a_stream_ungetc(&self->in);
      return false;
  }

  int64_t v = 0;
  int n = 0;
  
  while ((c = a_stream_getc(&self->in))) {
    if (!isdigit(c)) {
      a_stream_ungetc(&self->in);
      break;
    }
    
    v *= 10;
    v += c - '0';
    self->pos.column++;
    n++;
  }

  if (!n) { return NULL; }
  int scale = self->pos.column - fpos.column - 1;
  struct a_form *f = a_parser_push(self, A_LIT_FORM, fpos);
  if (trunc < 0) { v = -v; }
  a_val_init(&f->as_lit.val, &self->vm->math.fix_type)->as_fix = a_fix_new(trunc*a_pow(scale) + v, scale);
  return f;  
}

struct a_form *a_parse_id(struct a_parser *self) {
  struct a_pos fpos = self->pos;
  struct a_stream out;
  a_stream_init(&out);
  char c = 0;

  while ((c = a_stream_getc(&self->in))) {
    if (isspace(c) ||
	c == '(' || c == ')' || c == '[' || c == ']' || c == ':' || c == '.' || c == '\'') {
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
  int64_t v = 0;
  bool neg = false;
  struct a_pos fpos = self->pos;
  
  char c = a_stream_getc(&self->in);
  if (!c) { return NULL; }

  if (c == '-') {
    c = a_stream_getc(&self->in);

    if (isdigit(c)) {
      neg = true;
      self->pos.column++;
      a_stream_ungetc(&self->in);
    } else {
      a_stream_ungetc(&self->in);
      a_stream_ungetc(&self->in);
      return NULL;
    }
  } else {
      a_stream_ungetc(&self->in);    
  }

  int n = 0;
  
  while ((c = a_stream_getc(&self->in))) {
    if (c == '.') {
      c = a_stream_getc(&self->in);
      a_stream_ungetc(&self->in);
	
      if (c) {
	a_stream_ungetc(&self->in);
	if (isdigit(c)) { return a_parse_fix(self, neg ? -v : v); }
      }
      
      break;
    }
    
    if (!isdigit(c)) {
      a_stream_ungetc(&self->in);
      break;
    }
    
    v *= 10;
    v += c - '0';
    self->pos.column++;
    n++;
  }

   if (!n) { return NULL; }
   struct a_form *f = a_parser_push(self, A_LIT_FORM, fpos);
   a_val_init(&f->as_lit.val, &self->vm->abc.int_type)->as_int = neg ? -v : v;
   return f;
}

struct a_form *a_parse_list(struct a_parser *self) {
  struct a_pos fpos = self->pos;
  if (!a_parser_check(self, '[')) { return NULL; }
  struct a_form *lsf = a_malloc(self->vm, sizeof(struct a_form));
  a_form_init(lsf, A_LIST_FORM, fpos);

  while (true) {
    a_skip_space(self);
    char c = a_stream_getc(&self->in);

    if (c == ']') {
      self->pos.column++;
      break;
    }

    if (c) { a_stream_ungetc(&self->in); }
    struct a_form *f = a_parser_pop_next(self);
      
    if (!f) {
      a_form_deref(lsf, self->vm);
      a_fail("Open ls form");
      return NULL;
    }

    a_ls_push(&lsf->as_list.items, &f->ls);
  }

  a_ls_push(&self->forms, &lsf->ls);
  return lsf;
}

struct a_form *a_parse_pair(struct a_parser *self) {
  if (!a_parser_check(self, ':')) { return NULL; }
  struct a_form *l = a_parser_pop_last(self);
  
  if (!l) {
    a_fail("Missing left value");
    return NULL;
  }

  struct a_form *r = a_parser_pop_next(self);

  if (!r) {
    a_fail("Missing right value");
    return NULL;
  }

  struct a_form *f = a_parser_push(self, A_PAIR_FORM, l->pos);
  f->as_pair.left = l;
  f->as_pair.right = r;
  return f;
}

struct a_form *a_parse_string(struct a_parser *self) {
  struct a_pos fpos = self->pos;
  if (!a_parser_check(self, '"')) { return NULL; }
  struct a_stream out;
  a_stream_init(&out);
  char c = 0;
  
  while ((c = a_stream_getc(&self->in))) {
    self->pos.column++;
    if (c == '"') { break; }
    a_stream_putc(&out, c);
  }

  if (c != '"') {
    a_fail("Open string");
    return NULL;
  }
  
  struct a_form *f = a_parser_push(self, A_LIT_FORM, fpos);
  a_val_init(&f->as_lit.val, &self->vm->abc.string_type)->as_string = a_string(self->vm, out.data);
  a_stream_deinit(&out);
  return f;
}

struct a_form *a_parse_quote(struct a_parser *self) {
  struct a_pos fpos = self->pos;
  if (!a_parser_check(self, '\'')) { return NULL; }
  struct a_form *f = a_parser_pop_next(self);

  if (!f) {
    a_fail("Missing quoted form");
    return NULL;
  }

  struct a_form *qf = a_parser_push(self, A_QUOTE_FORM, fpos);
  qf->as_quote.form = f;
  return qf;
}
