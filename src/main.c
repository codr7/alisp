#include <stdio.h>
#include "alisp/parser.h"
#include "alisp/parsers.h"
#include "alisp/stack.h"
#include "alisp/stream.h"
#include "alisp/string.h"
#include "alisp/vm.h"

int main() {
  printf("Welcome to aLisp v%d\n\n", A_VERSION);
  printf("Return on empty line evaluates,\n");
  printf("(reset) clears the stack and Ctrl+D exits.\n\n");
	 
  struct a_vm vm;
  a_vm_init(&vm);

  struct a_parser parser;
  a_parser_init(&parser, &vm, a_string(&vm, "repl"));
  a_parser_add_prefix(&parser, a_skip_space);
  a_parser_add_prefix(&parser, a_parse_int);
  a_parser_add_prefix(&parser, a_parse_call);
  a_parser_add_prefix(&parser, a_parse_ls);
  a_parser_add_prefix(&parser, a_parse_id);
  a_parser_add_suffix(&parser, a_parse_dot);
  a_parser_add_suffix(&parser, a_parse_pair);
  
  while (!feof(stdin)) {
    printf("  ");
    char *line = a_stream_getline(&parser.in, stdin);
    if (feof(stdin)) { break; }
    if (line[0] != '\n') { continue; }
    a_pc_t pc = a_pc(&vm);
    while (a_parser_next(&parser));
    struct a_form *f;
    
    while ((f = a_parser_pop(&parser))) {
      a_form_emit(f, &vm);
      a_form_deref(f, &vm);
    }
    
    if (a_pc(&vm) != pc) {
      a_emit(&vm, A_STOP_OP);
      a_eval(&vm, pc);
    }
    
    a_stack_dump(&vm.stack);
    printf("\n\n");
  }

  a_parser_deinit(&parser);
  a_vm_deinit(&vm);
  return 0;
}
