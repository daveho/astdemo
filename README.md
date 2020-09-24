# AST demo

Demonstrates two ways to create an Abstract Syntax Tree:

* recursively transform the parse tree
* have the parser build the AST directly

How to run

* `./astdemo -b` builds an AST by recursive transformation
* `./astdemo -2` builds an AST directly in the parser

Example input (input as standard input, or in a file):

```
a - b * 3 - 4 * c + 5
```

Expected AST (for both `-b` and `-2` options):

```
ADD
+--SUB
|  +--SUB
|  |  +--VARREF[a]
|  |  +--MULTIPLY
|  |     +--VARREF[b]
|  |     +--INT_LITERAL[3]
|  +--MULTIPLY
|     +--INT_LITERAL[4]
|     +--VARREF[c]
+--INT_LITERAL[5]
```

The `-p` option prints the parse tree.  Parse tree for example input:

```
E
+--T
|  +--F
|  |  +--IDENTIFIER[a]
|  +--T'
+--E'
   +--MINUS[-]
   +--T
   |  +--F
   |  |  +--IDENTIFIER[b]
   |  +--T'
   |     +--TIMES[*]
   |     +--F
   |     |  +--INTEGER_LITERAL[3]
   |     +--T'
   +--E'
      +--MINUS[-]
      +--T
      |  +--F
      |  |  +--INTEGER_LITERAL[4]
      |  +--T'
      |     +--TIMES[*]
      |     +--F
      |     |  +--IDENTIFIER[c]
      |     +--T'
      +--E'
         +--PLUS[+]
         +--T
         |  +--F
         |  |  +--INTEGER_LITERAL[5]
         |  +--T'
         +--E'
```
