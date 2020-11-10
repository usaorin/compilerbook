#include <ctype.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Type of token
typedef enum {
	TK_RESERVED, // Operator symbol
	TK_NUM,      // Integer
	TK_EOF,      // End of token
} token_type_t;

// Type of  Abstract Syntax Tree Node
typedef enum {
	ND_ADD, // +
	ND_SUB, // -
	ND_NEG, // unary -
	ND_MUL, // *
	ND_DIV, // /
	ND_EQ,  // ==
	ND_NE,  // !=
	ND_LT,  // <
	ND_LE,  // <=
	ND_NUM, // integer
} node_type_t;

typedef struct node node_t;
typedef struct token token_t;

// Abstract Syntax Tree Node
struct node {
	node_type_t type; // node type
	node_t *lhs;      // left-hand side
	node_t *rhs;      // right-hand side
	int val;          // Use when type is ND_NUM
};
// Token
struct token {
	token_type_t type; // Type of token
	token_t *next;     // Next input token
	int val;           // Value when type is TK_NUM
	char *str;         // Token string
	int len;           // Length of token
};

void error(char *fmt, ...);
void error_at(char *str, char *fmt, ...);
void error_tok(token_t *tok, char *fmt, ...);
bool equal(token_t *tok, char *op);
token_t *skip(token_t *tok, char *op);
token_t *tokenize(char *input);
void code_gen(node_t *node);
node_t *parse(token_t *tok);
