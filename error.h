#ifndef ERROR_H
#define ERROR_H

#include "node.h"

// Error reporting (for use by lexer, parser, semantic analyzer, etc...)

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

// Report an error at the source position indicated by given Node
void error_on_node(struct Node *node, const char *msg);

// Report an error at indicated source position
void error_at_pos(struct SourceInfo source_pos, const char *msg);

#ifdef __cplusplus
}
#endif // __cplusplus

#endif // ERROR_H
