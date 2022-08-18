#ifndef LEXER_H
#define LEXER_H

#include <stdio.h>
#include "token.h"
#include "node.h"

class Lexer {
private:
  FILE *m_in;
  Node *m_next;
  std::string m_filename;
  int m_line, m_col;
  bool m_eof;

public:
  Lexer(FILE *in, const std::string &filename);
  ~Lexer();

  Node *next();
  Node *peek();

  Location get_current_pos() const;

private:
  int read();
  void unread(int c);
  void fill();
  Node *read_token();
  Node *read_continued_token(enum TokenKind kind, const std::string &lexeme_start, int line, int col, int (*pred)(int));
  Node *token_create(enum TokenKind kind, const std::string &lexeme, int line, int col);
};

#endif // LEXER_H
