#include <stdio.h>
#include <unistd.h> // for getopt
#include "util.h"
#include "lexer.h"
#include "parser.h"
#include "interp.h"

enum {
  INTERPRET,
  PRINT_TOKENS,
  PRINT_PARSE_TREE,
};

int main(int argc, char **argv) {
  int mode = INTERPRET, opt;
  while ((opt = getopt(argc, argv, "lp")) != -1) {
    switch (opt) {
    case 'l':
      mode = PRINT_TOKENS;
      break;
    case 'p':
      mode = PRINT_PARSE_TREE;
      break;
    default:
      err_fatal("Unknown option: %c\n", opt);
    }
  }

  FILE *in;
  const char *filename;

  if (optind < argc) {
    // read input from file
    filename = argv[optind];
    in = fopen(filename, "r");
    if (!in) {
      err_fatal("Could not open input file '%s'\n", filename);
    }
  } else {
    filename = "<stdin>";
    in = stdin;
  }

  struct Lexer *lexer = lexer_create(in, filename);

  if (mode == PRINT_TOKENS) {
    int done = 0;
    while (!done) {
      struct Node *tok = lexer_next(lexer);
      if (!tok) {
        done = 1;
      } else {
        int kind = node_get_tag(tok);
        const char *lexeme = node_get_str(tok);
        printf("%d:%s\n", kind, lexeme);
        node_destroy(tok);
      }
    }
  } else {
    struct Parser *parser = parser_create(lexer);
    struct Node *root = parser_parse(parser);

    if (mode == PRINT_PARSE_TREE) {
      parser_print_parse_tree(root);
    } else {
      printf("TODO: interpret\n");
    }
  }

  return 0;
}
