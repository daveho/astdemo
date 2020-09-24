#include <stdlib.h>
#include "util.h"
#include "node.h"
#include "token.h"
#include "ast.h"
#include "parser.h" // for parse node tags
#include "buildast.h"

// Get an AST operator node tag from a token operator tag
int buildast_operator_tag(int op_tag) {
  switch (op_tag) {
  case TOK_PLUS:
    return AST_ADD;
  case TOK_MINUS:
    return AST_SUB;
  case TOK_TIMES:
    return AST_MULTIPLY;
  case TOK_DIVIDE:
    return TOK_DIVIDE;
  default:
    err_fatal("Unknown operator %d in parse tree\n", op_tag);
    return -1;
  }
}

// Build an AST for a production on E/E' or T/T', maintaining left
// associativity of operators.
//
// Parameters:
//   ast - the portion of the AST that has been built so far
//   right - parse tree continuation that may contain more operators
//           and expressions (E' or T')
static struct Node *buildast_left(struct Node *ast, struct Node *right) {
  if (node_get_num_kids(right) == 0) {
    // done with expression
    return ast;
  }

  // first child of right parse tree is the operator
  struct Node *op = node_get_kid(right, 0);
  int op_tag = node_get_tag(op);

  // second child is an operand (T or F), convert it to AST
  struct Node *term_ast = buildast(node_get_kid(right, 1));

  // join current additive expression AST with new term
  int ast_tag = buildast_operator_tag(op_tag);
  ast = node_build2(ast_tag, ast, term_ast);

  // continue recursively
  return buildast_left(ast, node_get_kid(right, 2));
}

struct Node *buildast(struct Node *t) {
  int tag = node_get_tag(t);

  switch (tag) {
  case NODE_E:
  case NODE_T:
    return buildast_left(buildast(node_get_kid(t, 0)), node_get_kid(t, 1));

  case NODE_F:
    return buildast(node_get_kid(t, node_get_tag(t) == TOK_LPAREN ? 1 : 0));

  case TOK_IDENTIFIER:
    return node_alloc_str_copy(AST_VARREF, node_get_str(t));

  case TOK_INTEGER_LITERAL:
    {
       const char *lexeme = node_get_str(t);
       struct Node *ast = node_alloc_str_copy(AST_INT_LITERAL, lexeme);
       node_set_ival(ast, atol(lexeme));
       return ast;
    }

  default:
    err_fatal("Unknown parse node type %d\n", tag);
    return NULL;
  }
}
