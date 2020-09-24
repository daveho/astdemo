#include <cctype>
#include <string>
#include "cpputil.h"
#include "token.h"
#include "error.h"
#include "lexer.h"

////////////////////////////////////////////////////////////////////////
// Lexer implementation
////////////////////////////////////////////////////////////////////////

struct Lexer {
private:
  FILE *m_in;
  struct Node *m_next;
  std::string m_filename;
  int m_line, m_col;
  bool m_eof;

public:
  Lexer(FILE *in, const std::string &filename);
  ~Lexer();

  struct Node *next();
  struct Node *peek();

  struct SourceInfo get_current_pos() const;

private:
  int read();
  void unread(int c);
  void fill();
  struct Node *read_token();
  struct Node *read_continued_token(enum TokenKind kind, const std::string &lexeme_start, int line, int col, int (*pred)(int));
  struct Node *token_create(enum TokenKind kind, const std::string &lexeme, int line, int col);
};

Lexer::Lexer(FILE *in, const std::string &filename)
  : m_in(in)
  , m_next(nullptr)
  , m_filename(filename)
  , m_line(1)
  , m_col(1)
  , m_eof(false) {
}

Lexer::~Lexer() {
}

struct Node *Lexer::next() {
  fill();
  Node *tok = m_next;
  m_next = nullptr;
  return tok;
}

struct Node *Lexer::peek() {
  fill();
  return m_next;
}

struct SourceInfo Lexer::get_current_pos() const {
  SourceInfo source_pos = {
    .filename = m_filename.c_str(),
    .line = m_line,
    .col = m_col,
  };
  return source_pos;
}

// Read the next character of input, returning -1 (and setting m_eof to true)
// if the end of input has been reached.
int Lexer::read() {
  if (m_eof) {
    return -1;
  }
  int c = fgetc(m_in);
  if (c < 0) {
    m_eof = true;
  } else if (c == '\n') {
    m_col = 1;
    m_line++;
  } else {
    m_col++;
  }
  return c;
}

// "Unread" a character.  Useful for when reading a character indicates
// that the current token has ended and the next one has begun.
void Lexer::unread(int c) {
  ungetc(c, m_in);
  m_col--;
}

void Lexer::fill() {
  if (!m_eof && !m_next) {
    m_next = read_token();
  }
}

struct Node *Lexer::read_token() {
  int c, line = -1, col = -1;

  // skip whitespace characters until a non-whitespace character is read
  for (;;) {
    line = m_line;
    col = m_col;
    c = read();
    if (c < 0 || !isspace(c)) {
      break;
    }
  }

  if (c < 0) {
    // reached end of file
    return nullptr;
  }

  std::string lexeme;
  lexeme.push_back(char(c));

  if (isalpha(c)) {
    return read_continued_token(TOK_IDENTIFIER, lexeme, line, col, isalpha);
  } else if (isdigit(c)) {
    return read_continued_token(TOK_INTEGER_LITERAL, lexeme, line, col, isdigit);
  } else {
    switch (c) {
    case '+':
      return token_create(TOK_PLUS, lexeme, line, col);
    case '-':
      return token_create(TOK_MINUS, lexeme, line, col);
    case '*':
      return token_create(TOK_TIMES, lexeme, line, col);
    case '/':
      return token_create(TOK_DIVIDE, lexeme, line, col);
    case '(':
      return token_create(TOK_LPAREN, lexeme, line, col);
    case ')':
      return token_create(TOK_RPAREN, lexeme, line, col);
    default:
      {
        struct SourceInfo pos = {
          .filename = m_filename.c_str(),
          .line = line,
          .col = col,
        };
        std::string errmsg = cpputil::format("Unrecognized character '%c'", c).c_str();
        error_at_pos(pos, errmsg.c_str());
        return nullptr;
      }
    }
  } 
}

// Read the continuation of a (possibly) multi-character token, such as
// an identifier or integer literal.  pred is a pointer to a predicate
// function to determine which characters are valid continuations.
struct Node *Lexer::read_continued_token(enum TokenKind kind, const std::string &lexeme_start, int line, int col, int (*pred)(int)) {
  std::string lexeme(lexeme_start);
  for (;;) {
    int c = read();
    if (c >= 0 && pred(c)) {
      // token has finished
      lexeme.push_back(char(c));
    } else {
      if (c >= 0) {
        unread(c);
      }
      return token_create(kind, lexeme, line, col);
    }
  }
}

// Helper function to create a Node object to represent a token.
struct Node *Lexer::token_create(enum TokenKind kind, const std::string &lexeme, int line, int col) {
  struct Node *token = node_alloc_str_copy(kind, lexeme.c_str());
  struct SourceInfo source_info = {
    .filename = m_filename.c_str(),
    .line = line,
    .col = col,
  };
  node_set_source_info(token, source_info);
  return token;
}

////////////////////////////////////////////////////////////////////////
// Lexer API functions
////////////////////////////////////////////////////////////////////////

struct Lexer *lexer_create(FILE *in, const char *filename) {
  return new Lexer(in, filename);
}

void lexer_destroy(struct Lexer *lexer) {
  delete lexer;
}

struct Node *lexer_next(struct Lexer *lexer) {
  return lexer->next();
}

struct Node *lexer_peek(struct Lexer *lexer) {
  return lexer->peek();
}

struct SourceInfo lexer_get_current_pos(struct Lexer *lexer) {
  return lexer->get_current_pos();
}
