#ifndef PARSER_H
#define PARSER_H

#include "lexer.h"
#include "node.h"

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

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

struct Parser;

struct Parser *parser_create(struct Lexer *lexer_to_adopt);
void parser_destroy(struct Parser *parser);

struct Node *parser_parse(struct Parser *parser);

void parser_print_parse_tree(struct Node *root);

#ifdef __cplusplus
}
#endif // __cplusplus

#endif // PARSER_H
