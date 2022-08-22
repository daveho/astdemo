#ifndef PARSER_H
#define PARSER_H

#include "lexer.h"
#include "node.h"
#include "treeprint.h"

// Enumeration to define the nonterminal symbols:
// these should have different integer values than
// the members of the TokenKind enumeration (i.e., so they
// can be distinguished from terminal symbols)
enum Nonterminal {
  NODE_E = 1000,
  NODE_EPrime,
  NODE_T,
  NODE_TPrime,
  NODE_F,
};

class ParserTreePrint : public TreePrint {
public:
  ParserTreePrint();
  virtual ~ParserTreePrint();

  virtual std::string node_tag_to_string(int tag) const;
};

class Parser {
private:
  struct Lexer *m_lexer;
  Node *m_next;

public:
  Parser(Lexer *lexer_to_adopt);
  ~Parser();

  Node *parse();

private:
  // Parse functions for nonterminal grammar symbols
  Node *parse_E();
  Node *parse_EPrime();
  Node *parse_T();
  Node *parse_TPrime();
  Node *parse_F();

  // Consume a specific token, wrapping it in a Node
  Node *expect(enum TokenKind tok_kind);

  // Report an error at current lexer position
  void error_at_current_loc(const std::string &msg);
};

#endif // PARSER_H
