#ifndef LEXER_H
#define LEXER_H

#include <stdio.h>
#include "node.h"

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

struct Lexer;

struct Lexer *lexer_create(FILE *in, const char *filename);
void lexer_destroy(struct Lexer *lexer);

struct Node *lexer_next(struct Lexer *lexer);
struct Node *lexer_peek(struct Lexer *lexer);
struct SourceInfo lexer_get_current_pos(struct Lexer *lexer);

#ifdef __cplusplus
}
#endif // __cplusplus

#endif // LEXER_H
