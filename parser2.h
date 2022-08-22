#ifndef PARSER2_H
#define PARSER2_H

#include "lexer.h"
#include "node.h"

class Parser2 {
private:
  Lexer *m_lexer;
  Node *m_next;

public:
  Parser2(Lexer *lexer_to_adopt);
  ~Parser2();

  Node *parse();

private:
  // Parse functions for nonterminal grammar symbols
  Node *parse_E();
  Node *parse_EPrime(Node *ast);
  Node *parse_T();
  Node *parse_TPrime(Node *ast);
  Node *parse_F();

  // Consume a specific token, wrapping it in a Node
  Node *expect(enum TokenKind tok_kind);

  // Consume a specific token and discard it
  void expect_and_discard(enum TokenKind tok_kind);

  // Report an error at current lexer position
  void error_at_current_loc(const std::string &msg);
};

#endif // PARSER2_H
