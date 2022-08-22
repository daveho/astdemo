#include <stdio.h>
#include <unistd.h> // for getopt
#include <memory>
#include "lexer.h"
#include "parser.h"
#include "parser2.h"
#include "ast.h"
#include "buildast.h"
#include "exceptions.h"
#include "treeprint.h"

enum {
  PRINT_TOKENS,
  PRINT_PARSE_TREE,
  BUILD_AST,
  PARSER2,
};


int execute(int argc, char **argv) {
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
      RuntimeError::raise("Unknown option: %c", opt);
    }
  }

  FILE *in;
  const char *filename;

  if (optind < argc) {
    // read input from file
    filename = argv[optind];
    in = fopen(filename, "r");
    if (!in) {
      RuntimeError::raise("Could not open input file '%s'", filename);
    }
  } else {
    filename = "<stdin>";
    in = stdin;
  }

  Lexer *lexer = new Lexer(in, filename);

  if (mode == PRINT_TOKENS) {
    bool done = false;
    while (!done) {
      Node *tok = lexer->next();
      if (!tok) {
        done = true;
      } else {
        int kind = tok->get_tag();
        std::string lexeme = tok->get_str();
        printf("%d:%s\n", kind, lexeme.c_str());
        delete tok;
      }
    }
  } else if (mode == PRINT_PARSE_TREE || mode == BUILD_AST) {
    std::unique_ptr<Parser> parser(new Parser(lexer));
    std::unique_ptr<Node> root(parser->parse());

    if (mode == PRINT_PARSE_TREE) {
      ParserTreePrint tp;
      tp.print(root.get());
    } else {
      std::unique_ptr<Node> ast(buildast(root.get()));
      ASTTreePrint tp;
      tp.print(ast.get());
    }
  } else {
    std::unique_ptr<Parser2> parser2(new Parser2(lexer));
    std::unique_ptr<Node> ast(parser2->parse());
    ASTTreePrint tp;
    tp.print(ast.get());
  }

  return 0;
}

int main(int argc, char **argv) {
  try {
    return execute(argc, argv);
  } catch (BaseException &ex) {
    if (ex.has_location()) {
      const Location &loc = ex.get_loc();
      fprintf(stderr, "%s:%d: Error: %s\n", loc.get_srcfile().c_str(), loc.get_line(), ex.what());
    } else {
      fprintf(stderr, "Error: %s\n", ex.what());
    }
    return 1;
  }
}
