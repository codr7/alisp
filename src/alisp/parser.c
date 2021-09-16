#include "alisp/parser.h"
#include "alisp/parsers.h"
#include "alisp/vm.h"

struct a_parser *a_parser_init(struct a_parser *self, struct a_vm *vm, struct a_string *source) {
  self->vm = vm;
  self->pos = a_pos(source, A_MIN_LINE, A_MIN_COLUMN);
  a_ls_init(&self->prefix);
  a_ls_init(&self->suffix);
  a_ls_init(&self->forms);
  a_stream_init(&self->in);

  a_parser_add_prefix(self, a_skip_space);
  a_parser_add_prefix(self, a_parse_int);
  a_parser_add_prefix(self, a_parse_call);
  a_parser_add_prefix(self, a_parse_string);  
  a_parser_add_prefix(self, a_parse_list);
  a_parser_add_prefix(self, a_parse_quote);
  a_parser_add_prefix(self, a_parse_id);
  a_parser_add_suffix(self, a_parse_dot);
  a_parser_add_suffix(self, a_parse_pair);

  return self;
}

void a_parser_deinit(struct a_parser *self) {
  a_ls_do(&self->prefix, ls) {
    a_free(self->vm, a_baseof(ls, struct a_form_parser, ls));
  }

  a_ls_do(&self->suffix, ls) {
    a_free(self->vm, a_baseof(ls, struct a_form_parser, ls));
  }

  a_ls_do(&self->forms, ls) {
    a_form_deref(a_baseof(ls, struct a_form, ls), self->vm);
  }
}

struct a_form_parser *a_parser_add_prefix(struct a_parser *self, a_parser_body body) {
  struct a_form_parser *fp = a_malloc(self->vm, sizeof(struct a_form_parser));
  a_ls_push(&self->prefix, &fp->ls);
  fp->body = body;
  return fp;
}

struct a_form_parser *a_parser_add_suffix(struct a_parser *self, a_parser_body body) {
  struct a_form_parser *fp = a_malloc(self->vm, sizeof(struct a_form_parser));
  a_ls_push(&self->suffix, &fp->ls);
  fp->body = body;
  return fp;
}

struct a_form *a_parser_push(struct a_parser *self, enum a_form_type type, struct a_pos pos) {
  struct a_form *f = a_form_new(self->vm, type, pos);
  a_ls_push(&self->forms, &f->ls);
  return f;
}

struct a_form *a_parser_pop(struct a_parser *self) {  
  struct a_ls *fls = self->forms.next;
  if (fls == &self->forms) { return NULL; }
  return a_baseof(a_ls_pop(fls), struct a_form, ls);
}

struct a_form *a_parser_pop_last(struct a_parser *self) {
  struct a_ls *fls = self->forms.prev;
  if (fls == &self->forms) { return NULL; }
  return a_baseof(a_ls_pop(fls), struct a_form, ls);
}

bool a_parser_next(struct a_parser *self) {
  a_ls_do(&self->prefix, pls) {
    struct a_form_parser *p = a_baseof(pls, struct a_form_parser, ls);
    struct a_form *pf = p->body(self);

    if (pf) {
    read_suffix:
      a_ls_do(&self->suffix, sls) {
	struct a_form_parser *s = a_baseof(sls, struct a_form_parser, ls);
	struct a_form *sf = s->body(self);
	if (sf) { goto read_suffix; }
      }

      if (self->in.rpos == self->in.length) { a_stream_reset(&self->in); }
      return true;
    }
  }
  
  return false;
}

struct a_form *a_parser_peek_next(struct a_parser *self) {
  if (!a_parser_next(self)) { return NULL; }
  struct a_ls *fls = self->forms.prev;
  if (fls == &self->forms) { return NULL; }
  return a_baseof(fls, struct a_form, ls);
}

struct a_form *a_parser_pop_next(struct a_parser *self) {
  struct a_form *f = a_parser_peek_next(self);
  if (!f) { return NULL; }
  a_ls_pop(&f->ls);
  return f;
}

bool a_parser_check(struct a_parser *self, char it) {
  char c = a_stream_getc(&self->in);
  if (!c) { return false; }
  
  if (c != it) {
      a_stream_ungetc(&self->in);
      return false;
  }

  self->pos.column++;
  return true;
}
