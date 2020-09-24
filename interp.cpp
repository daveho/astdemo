#include <string>
#include <map>
#include <cassert>
#include "util.h"
#include "cpputil.h"
#include "token.h"
#include "error.h"
#include "interp.h"

////////////////////////////////////////////////////////////////////////
// Interpreter implementation
////////////////////////////////////////////////////////////////////////

struct Interpreter {
private:
  struct Node *m_tree;
  std::map<std::string, long> m_vars;

public:
  Interpreter(struct Node *tree);
  ~Interpreter();

  long exec();

private:
  long eval(struct Node *expr);
};

Interpreter::Interpreter(struct Node *tree) : m_tree(tree) {
}

Interpreter::~Interpreter() {
}

long Interpreter::exec() {
  long result = -1;
  struct Node *unit = m_tree;

  while (unit) {
    // first child is (E)xpression
    struct Node *expr = node_get_kid(unit, 0);

    // evaluate the expression!
    result = eval(expr);

    // if there are more expressions, the next (U)nit
    // is the third child
    if (node_get_num_kids(unit) == 3) {
      unit = node_get_kid(unit, 2);
    } else {
      // no more expressions
      unit = nullptr;
    }
  }

  return result;
}

long Interpreter::eval(struct Node *expr) {
  // the number of children and the first child's tag will determine
  // how to evaluate the expression
  struct Node *first = node_get_kid(expr, 0);
  int num_kids = node_get_num_kids(expr);
  int tag = node_get_tag(first);

  if (num_kids == 1) {
    // leaf expression (either an integer literal or identifier)
    const char *lexeme = node_get_str(first);
    if (tag == TOK_INTEGER_LITERAL) {
      // convert lexeme to an integer value
      return strtol(lexeme, nullptr, 10);
    } else {
      // look up value of variable
      assert(tag == TOK_IDENTIFIER);
      std::map<std::string, long>::const_iterator i = m_vars.find(lexeme);
      if (i == m_vars.end()) {
        std::string errmsg = cpputil::format("Undefined variable '%s'", lexeme);
        error_on_node(expr, errmsg.c_str());
      }
      return i->second;
    }
  }

  // left and right operands follow
  struct Node *left = node_get_kid(expr, 1);
  struct Node *right = node_get_kid(expr, 2);

  // Do the evaluation
  switch (tag) {
  case TOK_PLUS:
    return eval(left) + eval(right);
  case TOK_MINUS:
    return eval(left) - eval(right);
  case TOK_TIMES:
    return eval(left) * eval(right);
  case TOK_DIVIDE:
    return eval(left) / eval(right);

  case TOK_ASSIGN:
    // in this case, the left operand is an identifier naming
    // the variable
    {
      // get the variable name
      std::string varname = node_get_str(left);
      // evaluate the expression producing the value to be assigned
      long rvalue = eval(right);
      // store the value
      m_vars[varname] = rvalue;
      // result of the evaluation is the value assigned
      return rvalue;
    }

  default:
    err_fatal("Unknown operator: %d\n", tag);
    return -1L;
  }
}

////////////////////////////////////////////////////////////////////////
// Interpreter API functions
////////////////////////////////////////////////////////////////////////

struct Interpreter *interp_create(struct Node *tree) {
  return new Interpreter(tree);
}

void interp_destroy(struct Interpreter *interp) {
  delete interp;
}

long interp_exec(struct Interpreter *interp) {
  return interp->exec();
}
