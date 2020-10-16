#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <stdarg.h>
#include <stdbool.h>
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
	ND_MUL, // *
	ND_DIV, // /
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
};

// Current target token
token_t *cur_token;
// Input program
char *user_input;

// Output error
void error(char *fmt, ...) {
	va_list ap;
	va_start(ap, fmt);
	vfprintf(stderr, fmt, ap);
	fprintf(stderr, "\n");
	exit(1);
}

// Report error position
void error_at(char *loc, char *fmt, ...) {
	va_list ap;
	va_start(ap, fmt);

	int pos = loc - user_input;
	fprintf(stderr, "%s\n", user_input);
	fprintf(stderr, "%*s", pos, "");
	fprintf(stderr, "^ ");
	fprintf(stderr, fmt, ap);
	fprintf(stderr, "\n");
	exit(1);
}

// Create new node
node_t *new_node(node_type_t type, node_t *lhs, node_t *rhs) {
	node_t *node = calloc(1, sizeof(node_t));
	node->type = type;
	node->lhs = lhs;
	node->rhs = rhs;
	return node;
}

// Create new number node
node_t *new_node_num(int val) {
	node_t *node = calloc(1, sizeof(node_t));
	node->type = ND_NUM;
	node->val = val;
	return node;
}

// Create new token.
token_t *new_token(token_type_t type, token_t *cur, char *str) {
	token_t *tok = calloc(1, sizeof(token_t));
	tok->type = type;
	tok->str = str;
	cur->next = tok;
	return tok;
}

// Check expect operator symbol and consume a token.
// Return true if the next token is the expected token, otherwise false.
bool consume(char op) {
	if (cur_token->type != TK_RESERVED || cur_token->str[0] != op)
		return false;
	cur_token = cur_token->next;
	return true;
}

// Check expect operator symbol and consume a token.
// Output an error if it is not the expected symbol.
void expect_op(char op) {
	if (cur_token->type != TK_RESERVED || cur_token->str[0] != op)
		error_at(cur_token->str, "Expected operator symbol.");
	cur_token = cur_token->next;
}

// Check expect number and consume a token.
// Return the number if the next token is the expected token, otherwise errorno(-1).
int expect_number() {
	if (cur_token->type != TK_NUM)
		error_at(cur_token->str, "Expected a number.");
	int val = cur_token->val;
	cur_token = cur_token->next;
	return val;
}

bool at_eof() {
	return cur_token->type == TK_EOF;
}

node_t *expr();

// primary = "(" expr ")" | num
node_t *primary() {
	// "(" expr ")"
	if (consume('(')) {
		node_t *node = expr();
		expect_op(')');
		return node;
	}
	// num
	return new_node_num(expect_number());
}
// mul = primary ("*" primary | "/" primary)*
node_t *mul() {
	node_t *node = primary();

	for (;;) {
		if (consume('*'))
			node = new_node(ND_MUL, node, primary());
		else if (consume('/'))
			node = new_node(ND_DIV, node, primary());
		else
			return node;
	}
}

// expr = mul ("+" mul | "-" mul)*
node_t *expr() {
	node_t *node = mul();

	for (;;) {
		if (consume('+'))
			node = new_node(ND_ADD, node, mul());
		else if (consume('-'))
			node = new_node(ND_SUB, node, mul());
		else
			return node;
	}
}

// Stack operation
void gen(node_t *node) {
	if (node->type == ND_NUM) {
		printf("  push %d\n", node->val);
		return;
	}

	gen(node->lhs);
	gen(node->rhs);

	printf("  pop rdi\n");
	printf("  pop rax\n");

	switch (node->type) {
	case ND_ADD:
		printf("  add rax, rdi\n");
		break;
	case ND_SUB:
		printf("  sub rax, rdi\n");
		break;
	case ND_MUL:
		printf("  imul rax, rdi\n");
		break;
	case ND_DIV:
		printf("  cqo\n");
		printf("  idiv rdi\n");
		break;
	}

	printf("  push rax\n");
}

// Tokenize the target string
token_t *tokenize() {
	char *p = user_input;
	token_t head;
	head.next = NULL;
	token_t *cur = &head;

	while (*p) {
		// Skip space
		if (isspace(*p)) {
			p++;
			continue;
		}

		if (strchr("+-*/()", *p)) {
			cur = new_token(TK_RESERVED, cur, p++);
			continue;
		}

		if (isdigit(*p)) {
			cur = new_token(TK_NUM, cur, p);
			cur->val = strtol(p, &p, 10);
			continue;
		}

		error_at(p, "Invalid token.");
	}

	new_token(TK_EOF, cur, p);
	return head.next;
}


int
main(int argc, char **argv) {
	if (argc != 2)
		error("The number of arguments is incorrect.\n");

	// Tokenize and parse.
	user_input = argv[1];
	cur_token = tokenize(user_input);
	node_t *node = expr();

	// Outputs the first part of the assembly.
	printf(".intel_syntax noprefix\n");
	printf(".globl main\n");
	printf("main:\n");

	// Generate code from AST
	gen(node);

	// Pop from the top of the stack and put it in rax.
	printf("  pop rax\n");
	printf("  ret\n");

	return 0;
}
