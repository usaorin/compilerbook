#include "mycc.h"

// Input program
char *user_input;

// Output error
void
error(char *fmt, ...) {
	va_list ap;
	va_start(ap, fmt);
	vfprintf(stderr, fmt, ap);
	fprintf(stderr, "\n");
	exit(1);
}

// Report error position
static void
verror_at(char *str, char *fmt, va_list ap) {
	int pos = str - user_input;
	fprintf(stderr, "%s\n", user_input);
	fprintf(stderr, "%*s", pos, "");
	fprintf(stderr, "^ ");
	fprintf(stderr, fmt, ap);
	fprintf(stderr, "\n");
	exit(1);
}

void
error_at(char *str, char *fmt, ...) {
	va_list ap;
	va_start(ap, fmt);
	verror_at(str, fmt, ap);
}

void
error_tok(token_t *tok, char *fmt, ...) {
	va_list ap;
	va_start(ap, fmt);
	verror_at(tok->str, fmt, ap);
}

bool
equal(token_t *tok, char *op) {
	return memcmp(tok->str, op, tok->len) == 0 && op[tok->len] == '\0';
}

token_t *
skip(token_t *tok, char *op) {
	if (!equal(tok, op)) error_tok(tok, "expected '%s'", op);
	return tok->next;
}

static bool
startswitch(char *p, char *q) {
	return memcmp(p, q, strlen(q)) == 0;
}

static int
punct_len(char *p) {
	if (startswitch(p, "==") || startswitch(p, "!=") || startswitch(p, "<=") ||
	    startswitch(p, ">="))
		return 2;
	return ispunct(*p) ? 1 : 0;
}

// Create new token.
static token_t *
new_token(token_type_t type, char *start, char *end) {
	token_t *tok = calloc(1, sizeof(token_t));
	tok->type    = type;
	tok->str     = start;
	tok->len     = end - start;
	return tok;
}

// Tokenize the target string
token_t *
tokenize(char *p) {
	user_input   = p;
	token_t head = {};
	token_t *cur = &head;

	while (*p) {
		// Skip space
		if (isspace(*p)) {
			p++;
			continue;
		}

		if (isdigit(*p)) {
			cur = cur->next = new_token(TK_NUM, p, p);
			char *q         = p;
			cur->val        = strtol(p, &p, 10);
			cur->len        = p - q;
			continue;
		}

		int len = punct_len(p);
		if (len) {
			cur = cur->next = new_token(TK_RESERVED, p, p + len);
			p += len;
			continue;
		}

		error_at(p, "Invalid token.");
	}

	cur = cur->next = new_token(TK_EOF, p, p);
	return head.next;
}
