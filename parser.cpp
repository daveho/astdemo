#include <string>
#include "util.h"
#include "cpputil.h"
#include "treeprint.h"
#include "token.h"
#include "error.h"
#include "parser.h"

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

struct Parser {
private:
  struct Lexer *m_lexer;
  struct Node *m_next;

public:
  Parser(Lexer *lexer_to_adopt);
  ~Parser();

  struct Node *parse();

private:
  // Parse functions for nonterminal grammar symbols
  struct Node *parse_E();
  struct Node *parse_EPrime();
  struct Node *parse_T();
  struct Node *parse_TPrime();
  struct Node *parse_F();

  // Consume a specific token, wrapping it in a Node
  struct Node *expect(enum TokenKind tok_kind);

  // Report an error at current lexer position
  void error_at_current_pos(const std::string &msg);
};

Parser::Parser(Lexer *lexer_to_adopt) : m_lexer(lexer_to_adopt), m_next(nullptr) {
}

Parser::~Parser() {
  lexer_destroy(m_lexer);
}

struct Node *Parser::parse() {
  // E is the start symbol
  return parse_E();
}

struct Node *Parser::parse_E() {
  struct Node *e = node_build0(NODE_E);
  // E -> ^ T E'
  node_add_kid(e, parse_T());
  node_add_kid(e, parse_EPrime());
  return e;
}

struct Node *Parser::parse_EPrime() {
  struct Node *eprime = node_build0(NODE_EPrime);

  // E' -> ^ + T E'
  // E' -> ^ - T E'
  // E' -> ^ epsilon

  // peek at next token
  struct Node *next_tok = lexer_peek(m_lexer);
  if (next_tok && node_get_tag(next_tok) == TOK_PLUS) {
    // E' -> ^ + T E'
    node_add_kid(eprime, expect(TOK_PLUS));
    node_add_kid(eprime, parse_T());
    node_add_kid(eprime, parse_EPrime());
  } else if (next_tok && node_get_tag(next_tok) == TOK_MINUS) {
    // E' -> ^ - T E'
    node_add_kid(eprime, expect(TOK_MINUS));
    node_add_kid(eprime, parse_T());
    node_add_kid(eprime, parse_EPrime());
  } else {
    // E' -> ^ epsilon
    // apply epsilon production (by doing nothing)
  }

  return eprime;
}

struct Node *Parser::parse_T() {
  struct Node *t = node_build0(NODE_T);
  // T -> F T'
  node_add_kid(t, parse_F());
  node_add_kid(t, parse_TPrime());
  return t;
}

struct Node *Parser::parse_TPrime() {
  struct Node *tprime = node_build0(NODE_TPrime);

  // T' -> ^ * F T'
  // T' -> ^ / F T'
  // T' -> ^ epsilon

  // peek at next token
  struct Node *next_tok = lexer_peek(m_lexer);
  if (next_tok && node_get_tag(next_tok) == TOK_TIMES) {
    // T' -> ^ * F T'
    node_add_kid(tprime, expect(TOK_TIMES));
    node_add_kid(tprime, parse_F());
    node_add_kid(tprime, parse_TPrime());
  } else if (next_tok && node_get_tag(next_tok) == TOK_DIVIDE) {
    // T' -> ^ / F T'
    node_add_kid(tprime, expect(TOK_DIVIDE));
    node_add_kid(tprime, parse_F());
    node_add_kid(tprime, parse_TPrime());
  } else {
    // T' -> ^ epsilon
    // apply epsilon production by doing nothing
  }

  return tprime;
}

struct Node *Parser::parse_F() {
  // F -> ^ n
  // F -> ^ i
  // F -> ^ ( E )

  struct Node *f = node_build0(NODE_F);

  struct Node *next_tok = lexer_peek(m_lexer);
  if (!next_tok) {
    error_at_current_pos("Unexpected end of input looking for primary expression");
  }

  int tag = node_get_tag(next_tok);
  if (tag == TOK_INTEGER_LITERAL) {
    // F -> ^ n
    node_add_kid(f, expect(TOK_INTEGER_LITERAL));
  } else if (tag == TOK_IDENTIFIER) {
    // F -> ^ i
    node_add_kid(f, expect(TOK_IDENTIFIER));
  } else if (tag == TOK_LPAREN) {
    // F -> ^ ( E )
    node_add_kid(f, expect(TOK_LPAREN));
    node_add_kid(f, parse_E());
    node_add_kid(f, expect(TOK_RPAREN));
  } else {
    error_on_node(next_tok, "Invalid primary expression");
  }

  return f;
}

struct Node *Parser::expect(enum TokenKind tok_kind) {
  struct Node *next_terminal = lexer_next(m_lexer);
  if (!next_terminal) {
    error_at_current_pos("Unexpected end of input");
  }
  if (node_get_tag(next_terminal) != tok_kind) {
    std::string errmsg = cpputil::format("Unexpected token '%s'", node_get_str(next_terminal));
    error_on_node(next_terminal, errmsg.c_str());
  }
  return next_terminal;
}

// This function translates token and parse node tags into strings
// for parse tree printing
const char *astdemo_stringify_node_tag(int tag) {
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
    err_fatal("Unknown node tag: %d\n", tag);
    return "<<UNKNOWN>>";
  }
}

void Parser::error_at_current_pos(const std::string &msg) {
  struct SourceInfo current_pos = lexer_get_current_pos(m_lexer);
  error_at_pos(current_pos, msg.c_str());
}

////////////////////////////////////////////////////////////////////////
// Parser API functions
////////////////////////////////////////////////////////////////////////

struct Parser *parser_create(struct Lexer *lexer_to_adopt) {
  return new Parser(lexer_to_adopt);
}

void parser_destroy(struct Parser *parser) {
  delete parser;
}

struct Node *parser_parse(struct Parser *parser) {
  return parser->parse();
}

void parser_print_parse_tree(struct Node *root) {
  treeprint(root, astdemo_stringify_node_tag);
}
