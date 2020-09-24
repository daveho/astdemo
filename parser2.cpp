#include <string>
#include "util.h"
#include "cpputil.h"
#include "token.h"
#include "error.h"
#include "ast.h"
#include "parser2.h"

////////////////////////////////////////////////////////////////////////
// Parser2 implementation
// This version of the parser builds an AST directly,
// rather than first building a parse tree.
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

struct Parser2 {
private:
  struct Lexer *m_lexer;
  struct Node *m_next;

public:
  Parser2(Lexer *lexer_to_adopt);
  ~Parser2();

  struct Node *parse();

private:
  // Parse functions for nonterminal grammar symbols
  struct Node *parse_E();
  struct Node *parse_EPrime(struct Node *ast);
  struct Node *parse_T();
  struct Node *parse_TPrime(struct Node *ast);
  struct Node *parse_F();

  // Consume a specific token, wrapping it in a Node
  struct Node *expect(enum TokenKind tok_kind);

  // Consume a specific token and discard it
  void expect_and_discard(enum TokenKind tok_kind);

  // Report an error at current lexer position
  void error_at_current_pos(const std::string &msg);
};

Parser2::Parser2(Lexer *lexer_to_adopt) : m_lexer(lexer_to_adopt), m_next(nullptr) {
}

Parser2::~Parser2() {
  lexer_destroy(m_lexer);
}

struct Node *Parser2::parse() {
  // E is the start symbol
  return parse_E();
}

struct Node *Parser2::parse_E() {
  // E -> ^ T E'

  // Get the AST corresponding to the term (T)
  struct Node *ast = parse_T();

  // Recursively continue the additive expression
  return parse_EPrime(ast);  
}

// This function is passed the "current" portion of the AST
// that has been built so far for the additive expression.
struct Node *Parser2::parse_EPrime(struct Node *ast) {
  // E' -> ^ + T E'
  // E' -> ^ - T E'
  // E' -> ^ epsilon

  // peek at next token
  struct Node *next_tok = lexer_peek(m_lexer);
  if (next_tok) {
    int next_tok_tag = node_get_tag(next_tok);
    if (next_tok_tag == TOK_PLUS || next_tok_tag == TOK_MINUS)  {
      // E' -> ^ + T E'
      // E' -> ^ - T E'
      struct Node *op = expect(static_cast<enum TokenKind>(next_tok_tag));

      // build AST for next term, incorporate into current AST
      struct Node *term_ast = parse_T();
      ast = node_build2(next_tok_tag == TOK_PLUS ? AST_ADD : AST_SUB, ast, term_ast);

      // copy source information from operator node
      node_set_source_info(ast, node_get_source_info(op));
      node_destroy(op);

      // continue recursively
      return parse_EPrime(ast);
    }
  }

  // E' -> ^ epsilon
  // No more additive operators, so just return the completed AST
  return ast;
}

struct Node *Parser2::parse_T() {
  // T -> F T'

  // Parse primary expression
  struct Node *ast = parse_F();

  // Recursively continue the multiplicative expression
  return parse_TPrime(ast);
}

struct Node *Parser2::parse_TPrime(struct Node *ast) {
  // T' -> ^ * F T'
  // T' -> ^ / F T'
  // T' -> ^ epsilon

  // peek at next token
  struct Node *next_tok = lexer_peek(m_lexer);
  if (next_tok) {
    int next_tok_tag = node_get_tag(next_tok);
    if (next_tok_tag == TOK_TIMES || next_tok_tag == TOK_DIVIDE)  {
      // T' -> ^ * F T'
      // T' -> ^ / F T'
      struct Node *op = expect(static_cast<enum TokenKind>(next_tok_tag));

      // build AST for next primary expression, incorporate into current AST
      struct Node *primary_ast = parse_F();
      ast = node_build2(next_tok_tag == TOK_TIMES ? AST_MULTIPLY : AST_DIVIDE, ast, primary_ast);

      // copy source information from operator node
      node_set_source_info(ast, node_get_source_info(op));
      node_destroy(op);

      // continue recursively
      return parse_TPrime(ast);
    }
  }

  // T' -> ^ epsilon
  // No more multiplicative operators, so just return the completed AST
  return ast;
}

struct Node *Parser2::parse_F() {
  // F -> ^ n
  // F -> ^ i
  // F -> ^ ( E )

  struct Node *next_tok = lexer_peek(m_lexer);
  if (!next_tok) {
    error_at_current_pos("Unexpected end of input looking for primary expression");
  }

  int tag = node_get_tag(next_tok);
  if (tag == TOK_INTEGER_LITERAL || tag == TOK_IDENTIFIER) {
    // F -> ^ n
    // F -> ^ i
    struct Node *tok = expect(static_cast<enum TokenKind>(tag));
    const char *lexeme = node_get_str(tok);
    int ast_tag = tag == TOK_INTEGER_LITERAL ? AST_INT_LITERAL : AST_VARREF;
    struct Node *ast = node_alloc_str_copy(ast_tag, lexeme);
    node_set_source_info(ast, node_get_source_info(tok));
    if (ast_tag == AST_INT_LITERAL) {
      node_set_ival(ast, atol(lexeme));
    }
    node_destroy(tok);
    return ast;
  } else if (tag == TOK_LPAREN) {
    // F -> ^ ( E )
    expect_and_discard(TOK_LPAREN);
    struct Node *ast = parse_E();
    expect_and_discard(TOK_RPAREN);
    return ast;
  } else {
    error_on_node(next_tok, "Invalid primary expression");
    return nullptr;
  }
}

struct Node *Parser2::expect(enum TokenKind tok_kind) {
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

void Parser2::expect_and_discard(enum TokenKind tok_kind) {
  struct Node *tok = expect(tok_kind);
  node_destroy(tok);
}

void Parser2::error_at_current_pos(const std::string &msg) {
  struct SourceInfo current_pos = lexer_get_current_pos(m_lexer);
  error_at_pos(current_pos, msg.c_str());
}

////////////////////////////////////////////////////////////////////////
// Parser2 API functions
////////////////////////////////////////////////////////////////////////

struct Parser2 *parser2_create(struct Lexer *lexer_to_adopt) {
  return new Parser2(lexer_to_adopt);
}

void parser2_destroy(struct Parser2 *parser) {
  delete parser;
}

struct Node *parser2_parse(struct Parser2 *parser) {
  return parser->parse();
}
