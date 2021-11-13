#include "util.h"
#include "ast.h"

const char *ast_get_name_for_tag(int tag) {
  switch (tag) {
  case AST_ADD:
    return "ADD";
  case AST_SUB:
    return "SUB";
  case AST_MULTIPLY:
    return "MULTIPLY";
  case AST_DIVIDE:
    return "DIVIDE";
  case AST_VARREF:
    return "VARREF";
  case AST_INT_LITERAL:
    return "INT_LITERAL";
  default:
    err_fatal("Unknown AST node type %d\n", tag);
    return "<<unknown>>";
  }
}
