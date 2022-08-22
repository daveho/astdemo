#include <string>
#include <memory>
#include "treeprint.h"
#include "token.h"
#include "exceptions.h"
#include "treeprint.h"
#include "parser.h"

////////////////////////////////////////////////////////////////////////
// ParserTreePrint implementation
////////////////////////////////////////////////////////////////////////

ParserTreePrint::ParserTreePrint() {
}

ParserTreePrint::~ParserTreePrint() {
}

// This function translates token and parse node tags into strings
// for parse tree printing
std::string ParserTreePrint::node_tag_to_string(int tag) const {
  switch (tag) {
  // terminal symbols:
  case TOK_IDENTIFIER:
    return "IDENTIFIER";
  case TOK_INTEGER_LITERAL:
    return "INTEGER_LITERAL";
  case TOK_PLUS:
    return "PLUS";
  case TOK_MINUS:
    return "MINUS";
  case TOK_TIMES:
    return "TIMES";
  case TOK_DIVIDE:
    return "DIVIDE";
  case TOK_LPAREN:
    return "LPAREN";
  case TOK_RPAREN:
    return "RPAREN";

  // nonterminal symbols:
  case NODE_E:
    return "E";
  case NODE_EPrime:
    return "E'";
  case NODE_T:
    return "T";
  case NODE_TPrime:
    return "T'";
  case NODE_F:
    return "F";

  default:
    RuntimeError::raise("Unknown node tag: %d", tag);
  }
}

////////////////////////////////////////////////////////////////////////
// Parser implementation
////////////////////////////////////////////////////////////////////////

// This is the grammar (E is the start symbol):
//
// E -> T E'
// E' -> + T E'
// E' -> - T E'
// E' -> epsilon
// T -> F T'
// T' -> * F T'
// T' -> / F T'
// T' -> epsilon
// F -> n
// F -> i
// F -> ( E )

Parser::Parser(Lexer *lexer_to_adopt) : m_lexer(lexer_to_adopt), m_next(nullptr) {
}

Parser::~Parser() {
  delete m_lexer;
}

Node *Parser::parse() {
  // E is the start symbol
  return parse_E();
}

Node *Parser::parse_E() {
  std::unique_ptr<Node> e(new Node(NODE_E));
  // E -> ^ T E'
  e->append_kid(parse_T());
  e->append_kid(parse_EPrime());
  return e.release();
}

Node *Parser::parse_EPrime() {
  std::unique_ptr<Node> eprime(new Node(NODE_EPrime));

  // E' -> ^ + T E'
  // E' -> ^ - T E'
  // E' -> ^ epsilon

  // peek at next token
  Node *next_tok = m_lexer->peek();
  if (next_tok && next_tok->get_tag() == TOK_PLUS) {
    // E' -> ^ + T E'
    eprime->append_kid(expect(TOK_PLUS));
    eprime->append_kid(parse_T());
    eprime->append_kid(parse_EPrime());
  } else if (next_tok && next_tok->get_tag() == TOK_MINUS) {
    // E' -> ^ - T E'
    eprime->append_kid(expect(TOK_MINUS));
    eprime->append_kid(parse_T());
    eprime->append_kid(parse_EPrime());
  } else {
    // E' -> ^ epsilon
    // apply epsilon production (by doing nothing)
  }

  return eprime.release();
}

Node *Parser::parse_T() {
  std::unique_ptr<Node> t(new Node(NODE_T));
  // T -> F T'
  t->append_kid(parse_F());
  t->append_kid(parse_TPrime());
  return t.release();
}

Node *Parser::parse_TPrime() {
  std::unique_ptr<Node> tprime(new Node(NODE_TPrime));

  // T' -> ^ * F T'
  // T' -> ^ / F T'
  // T' -> ^ epsilon

  // peek at next token
  Node *next_tok = m_lexer->peek();
  if (next_tok && next_tok->get_tag() == TOK_TIMES) {
    // T' -> ^ * F T'
    tprime->append_kid(expect(TOK_TIMES));
    tprime->append_kid(parse_F());
    tprime->append_kid(parse_TPrime());
  } else if (next_tok && next_tok->get_tag() == TOK_DIVIDE) {
    // T' -> ^ / F T'
    tprime->append_kid(expect(TOK_DIVIDE));
    tprime->append_kid(parse_F());
    tprime->append_kid(parse_TPrime());
  } else {
    // T' -> ^ epsilon
    // apply epsilon production by doing nothing
  }

  return tprime.release();
}

Node *Parser::parse_F() {
  // F -> ^ n
  // F -> ^ i
  // F -> ^ ( E )

  std::unique_ptr<Node> f(new Node(NODE_F));

  Node *next_tok = m_lexer->peek();
  if (!next_tok) {
    error_at_current_loc("Unexpected end of input looking for primary expression");
  }

  int tag = next_tok->get_tag();
  if (tag == TOK_INTEGER_LITERAL) {
    // F -> ^ n
    f->append_kid(expect(TOK_INTEGER_LITERAL));
  } else if (tag == TOK_IDENTIFIER) {
    // F -> ^ i
    f->append_kid(expect(TOK_IDENTIFIER));
  } else if (tag == TOK_LPAREN) {
    // F -> ^ ( E )
    f->append_kid(expect(TOK_LPAREN));
    f->append_kid(parse_E());
    f->append_kid(expect(TOK_RPAREN));
  } else {
    SyntaxError::raise(next_tok->get_loc(), "Invalid primary expression");
  }

  return f.release();
}

Node *Parser::expect(enum TokenKind tok_kind) {
  std::unique_ptr<Node> next_terminal(m_lexer->next());
  if (!next_terminal) {
    error_at_current_loc("Unexpected end of input");
  }
  if (next_terminal->get_tag() != tok_kind) {
    SyntaxError::raise(next_terminal->get_loc(), "Unexpected token '%s'", next_terminal->get_str().c_str());
  }
  return next_terminal.release();
}

void Parser::error_at_current_loc(const std::string &msg) {
  SyntaxError::raise(m_lexer->get_current_loc(), "%s", msg.c_str());
}
