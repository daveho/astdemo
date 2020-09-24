#ifndef PARSER2_H
#define PARSER2_H

#include "lexer.h"
#include "node.h"

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

struct Parser2;

struct Parser2 *parser2_create(struct Lexer *lexer_to_adopt);
void parser2_destroy(struct Parser2 *parser);

// This function returns an AST!
struct Node *parser2_parse(struct Parser2 *parser);

#ifdef __cplusplus
}
#endif // __cplusplus

#endif // PARSER2_H
