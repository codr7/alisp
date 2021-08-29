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

  struct a_form *cf = a_malloc(&self->vm->form_pool, sizeof(struct a_form));
  a_form_init(cf, A_CALL_FORM, fpos)->as_call.target = t;
  
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
      a_free(&self->vm->form_pool, cf);
      a_fail("Open call form");
      return NULL;
    }

    a_ls_push(&cf->as_call.args, &f->ls);
    cf->as_call.arg_count++;
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
      a_stream_ungetc(&self->in);
    } else {
      a_stream_ungetc(&self->in);
      a_stream_ungetc(&self->in);
      return NULL;
    }
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
   struct a_form *f = a_parser_push(self, A_LIT_FORM, fpos);
   a_val_init(&f->as_lit.val, &self->vm->abc.int_type)->as_int = neg ? -v : v;
   return f;
}

struct a_form *a_parse_ls(struct a_parser *self) {
  struct a_pos fpos = self->pos;
  if (!a_parser_check(self, '[')) { return NULL; }
  struct a_form *lsf = a_malloc(&self->vm->form_pool, sizeof(struct a_form));
  a_form_init(lsf, A_LS_FORM, fpos);

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
      a_free(&self->vm->form_pool, lsf);
      a_fail("Open ls form");
      return NULL;
    }

    a_ls_push(&lsf->as_ls.items, &f->ls);
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
