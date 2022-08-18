#include <string>
#include "token.h"
#include "ast.h"
#include "exceptions.h"
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

Parser2::Parser2(Lexer *lexer_to_adopt) : m_lexer(lexer_to_adopt), m_next(nullptr) {
}

Parser2::~Parser2() {
  delete m_lexer;
}

Node *Parser2::parse() {
  // E is the start symbol
  return parse_E();
}

Node *Parser2::parse_E() {
  // E -> ^ T E'

  // Get the AST corresponding to the term (T)
  Node *ast = parse_T();

  // Recursively continue the additive expression
  return parse_EPrime(ast);  
}

// This function is passed the "current" portion of the AST
// that has been built so far for the additive expression.
Node *Parser2::parse_EPrime(Node *ast) {
  // E' -> ^ + T E'
  // E' -> ^ - T E'
  // E' -> ^ epsilon

  // peek at next token
  Node *next_tok = m_lexer->peek();
  if (next_tok) {
    int next_tok_tag = next_tok->get_tag();
    if (next_tok_tag == TOK_PLUS || next_tok_tag == TOK_MINUS)  {
      // E' -> ^ + T E'
      // E' -> ^ - T E'
      Node *op = expect(static_cast<enum TokenKind>(next_tok_tag));

      // build AST for next term, incorporate into current AST
      Node *term_ast = parse_T();
      ast = new Node(next_tok_tag == TOK_PLUS ? AST_ADD : AST_SUB, {ast, term_ast});

      // copy source information from operator node
      ast->set_loc(op->get_loc());
      delete op;

      // continue recursively
      return parse_EPrime(ast);
    }
  }

  // E' -> ^ epsilon
  // No more additive operators, so just return the completed AST
  return ast;
}

Node *Parser2::parse_T() {
  // T -> F T'

  // Parse primary expression
  Node *ast = parse_F();

  // Recursively continue the multiplicative expression
  return parse_TPrime(ast);
}

Node *Parser2::parse_TPrime(Node *ast) {
  // T' -> ^ * F T'
  // T' -> ^ / F T'
  // T' -> ^ epsilon

  // peek at next token
  Node *next_tok = m_lexer->peek();
  if (next_tok) {
    int next_tok_tag = next_tok->get_tag();
    if (next_tok_tag == TOK_TIMES || next_tok_tag == TOK_DIVIDE)  {
      // T' -> ^ * F T'
      // T' -> ^ / F T'
      Node *op = expect(static_cast<enum TokenKind>(next_tok_tag));

      // build AST for next primary expression, incorporate into current AST
      Node *primary_ast = parse_F();
      ast = new Node(next_tok_tag == TOK_TIMES ? AST_MULTIPLY : AST_DIVIDE, {ast, primary_ast});

      // copy source information from operator node
      ast->set_loc(op->get_loc());
      delete op;

      // continue recursively
      return parse_TPrime(ast);
    }
  }

  // T' -> ^ epsilon
  // No more multiplicative operators, so just return the completed AST
  return ast;
}

Node *Parser2::parse_F() {
  // F -> ^ n
  // F -> ^ i
  // F -> ^ ( E )

  Node *next_tok = m_lexer->peek();
  if (!next_tok) {
    error_at_current_pos("Unexpected end of input looking for primary expression");
  }

  int tag = next_tok->get_tag();
  if (tag == TOK_INTEGER_LITERAL || tag == TOK_IDENTIFIER) {
    // F -> ^ n
    // F -> ^ i
    Node *tok = expect(static_cast<enum TokenKind>(tag));
    int ast_tag = tag == TOK_INTEGER_LITERAL ? AST_INT_LITERAL : AST_VARREF;
    Node *ast = new Node(ast_tag);
    ast->set_str(tok->get_str());
    ast->set_loc(tok->get_loc());
    delete tok;
    return ast;
  } else if (tag == TOK_LPAREN) {
    // F -> ^ ( E )
    expect_and_discard(TOK_LPAREN);
    Node *ast = parse_E();
    expect_and_discard(TOK_RPAREN);
    return ast;
  } else {
    SyntaxError::raise(next_tok->get_loc(), "Invalid primary expression");
    return nullptr;
  }
}

Node *Parser2::expect(enum TokenKind tok_kind) {
  Node *next_terminal = m_lexer->next();
  if (!next_terminal) {
    error_at_current_pos("Unexpected end of input");
  }
  if (next_terminal->get_tag() != tok_kind) {
    SyntaxError::raise(next_terminal->get_loc(), "Unexpected token '%s'", next_terminal->get_str().c_str());
  }
  return next_terminal;
}

void Parser2::expect_and_discard(enum TokenKind tok_kind) {
  Node *tok = expect(tok_kind);
  delete tok;
}

void Parser2::error_at_current_pos(const std::string &msg) {
  SyntaxError::raise(m_lexer->get_current_pos(), "%s", msg.c_str());
}
