#ifndef INTERP_H
#define INTERP_H

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

struct Node;
struct Interpreter;

// create an Interpreter from given parse tree
struct Interpreter *interp_create(struct Node *tree);

// destroy given Interpreter
void interp_destroy(struct Interpreter *interp);

// execute the Interpreter, returning the result of the last evaluation
long interp_exec(struct Interpreter *interp);

#ifdef __cplusplus
}
#endif // __cplusplus

#endif // INTERP_H
