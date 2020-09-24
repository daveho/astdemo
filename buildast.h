#ifndef BUILDAST_H
#define BUILDAST_H

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

struct Node;

// build AST from a parse tree
struct Node *buildast(struct Node *t);

#ifdef __cplusplus
}
#endif // __cplusplus

#endif // BUILDAST_H
