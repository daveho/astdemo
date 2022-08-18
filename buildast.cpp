#include <stdlib.h>
#include "node.h"
#include "token.h"
#include "ast.h"
#include "parser.h" // for parse node tags
#include "exceptions.h"
#include "buildast.h"

namespace {

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
    RuntimeError::raise("Unknown operator %d in parse tree", op_tag);
  }
}

// Build an AST for a production on E/E' or T/T', maintaining left
// associativity of operators.
//
// Parameters:
//   ast - the portion of the AST that has been built so far
//   right - parse tree continuation that may contain more operators
//           and expressions (E' or T')
Node *buildast_left(Node *ast, Node *right) {
  if (right->get_num_kids() == 0) {
    // done with expression
    return ast;
  }

  // first child of right parse tree is the operator
  Node *op = right->get_kid(0);
  int op_tag = op->get_tag();

  // second child is an operand (T or F), convert it to AST
  Node *operand_ast = buildast(right->get_kid(1));

  // join current expression AST with new operand
  int ast_tag = buildast_operator_tag(op_tag);
  ast = new Node(ast_tag, {ast, operand_ast});

  // continue recursively
  return buildast_left(ast, right->get_kid(2));
}

} // end anonymous namespace

Node *buildast(Node *t) {
  int tag = t->get_tag();

  switch (tag) {
  case NODE_E:
  case NODE_T: // restructure for left associativity
    return buildast_left(buildast(t->get_kid(0)), t->get_kid(1));

  case NODE_F: // parenthesized expression, identifier, or integer literal
    return buildast(t->get_kid(t->get_num_kids() == 3 ? 1 : 0));

  case TOK_IDENTIFIER: // variable reference
    return new Node(AST_VARREF, t->get_str());

  case TOK_INTEGER_LITERAL: // integer literal
    return new Node(AST_INT_LITERAL, t->get_str());

  default:
    RuntimeError::raise("Unknown parse node type %d", tag);
  }
}
