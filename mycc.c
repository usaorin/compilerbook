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

typedef struct token token_t;

struct token {
	token_type_t type; // Type of token
	token_t *next;     // Next input token
	int val;         // Value when type is TK_NUM
	char *str;       // Token string
};

// Current target token
token_t *cur_token;

// Output error
void error(char *fmt, ...) {
	va_list ap;
	va_start(ap, fmt);
	vfprintf(stderr, fmt, ap);
	fprintf(stderr, "\n");
	exit(1);
}

// Check expect operator symbol and consume a token.
// Return true if the next token is the expected token, otherwise false.
bool expect_op(char op) {
	if (cur_token->type != TK_RESERVED || cur_token->str[0] != op)
		return false;
	cur_token = cur_token->next;
	return true;
}

// Check expect number and consume a token.
// Return the number if the next token is the expected token, otherwise errorno(-1).
int expect_number() {
	if (cur_token->type != TK_NUM)
		return -1;
	int val = cur_token->val;
	cur_token = cur_token->next;
	return val;
}

bool at_eof() {
	return cur_token->type == TK_EOF;
}

// Create new token.
token_t *new_token(token_type_t type, token_t *cur, char *str) {
	token_t *tok = calloc(1, sizeof(token_t));
	tok->type = type;
	tok->str = str;
	cur->next = tok;
	return tok;
}

// Tokenize the target string
token_t *tokenize(char *p) {
	token_t head;
	head.next = NULL;
	token_t *cur = &head;

	while (*p) {
		// Skip space
		if (isspace(*p)) {
			p++;
			continue;
		}

		if (*p == '+' || *p == '-') {
			cur = new_token(TK_RESERVED, cur, p++);
			continue;
		}

		if (isdigit(*p)) {
			cur = new_token(TK_NUM, cur, p);
			cur->val = strtol(p, &p, 10);
			continue;
		}

		error("Can not tokenize.");
	}

	new_token(TK_EOF, cur, p);
	return head.next;
}

int
main(int argc, char **argv) {
	if (argc != 2) {
		fprintf(stderr, "The number of arguments is incorrect.\n");
		return 1;
	}

	cur_token = tokenize(argv[1]);

	// Outputs the first part of the assembly.
	printf(".intel_syntax noprefix\n");
	printf(".globl main\n");
	printf("main:\n");

	// Must start with a number
	int n = expect_number();
	if (n < 0) {
		error("Must start with a number.");
	}
	printf("  mov rax, %d\n", n);

	while (!at_eof()) {
		if (expect_op('+')) {
			printf("  add rax, %d\n", expect_number());
			continue;
		}

		if (expect_op('-')) {
			printf("  sub rax, %d\n", expect_number());
		}
	}

	printf("  ret\n");
	return 0;
}
