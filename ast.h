#ifndef AST_H
#define AST_H

// AST node tags
enum ASTKind {
  AST_ADD,
  AST_SUB,
  AST_MULTIPLY,
  AST_DIVIDE,
  AST_VARREF,
  AST_INT_LITERAL,
};

// stringify function for AST node tags
const char *ast_get_name_for_tag(int tag);

#endif // AST_H
