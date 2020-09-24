#include <stdio.h>
#include <unistd.h> // for getopt
#include "util.h"
#include "lexer.h"
#include "parser.h"
#include "ast.h"
#include "buildast.h"
#include "treeprint.h"

enum {
  PRINT_TOKENS,
  PRINT_PARSE_TREE,
  BUILD_AST,
  PARSER2,
};

int main(int argc, char **argv) {
  int mode = PRINT_PARSE_TREE, opt;
  while ((opt = getopt(argc, argv, "lpb2")) != -1) {
    switch (opt) {
    case 'l':
      mode = PRINT_TOKENS;
      break;
    case 'p':
      mode = PRINT_PARSE_TREE;
      break;
    case 'b':
      mode = BUILD_AST;
      break;
    case '2':
      mode = PARSER2;
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
  } else if (mode == PRINT_PARSE_TREE || mode == BUILD_AST) {
    struct Parser *parser = parser_create(lexer);
    struct Node *root = parser_parse(parser);

    if (mode == PRINT_PARSE_TREE) {
      parser_print_parse_tree(root);
    } else {
      struct Node *ast = buildast(root);
      treeprint(ast, ast_get_name_for_tag);
    }
  } else {
    printf("TODO: build AST directly in parser\n");
  }

  return 0;
}
