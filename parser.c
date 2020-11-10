#include "mycc.h"

// Create new number node
static node_t *
new_num(int val) {
	node_t *node = calloc(1, sizeof(node_t));
	node->type   = ND_NUM;
	node->val    = val;
	return node;
}

// Create new node
static node_t *
new_node(node_type_t type, node_t *lhs, node_t *rhs) {
	node_t *node = calloc(1, sizeof(node_t));
	node->type   = type;
	node->lhs    = lhs;
	node->rhs    = rhs;
}

static node_t *expr(token_t **rest, token_t *tok);

// primary = "(" expr ")" | num
static node_t *
primary(token_t **rest, token_t *tok) {
	node_t *node;
	// "(" expr ")"
	if (equal(tok, "(")) {
		node  = expr(&tok, tok->next);
		*rest = skip(tok, ")");
	}
	// num
	if (tok->type == TK_NUM) {
		node  = new_num(tok->val);
		*rest = tok->next;
	}
	return node;
}

// unary = ("+" | "-")? primary
static node_t *
unary(token_t **rest, token_t *tok) {
	if (equal(tok, "+")) return unary(rest, tok->next);
	if (equal(tok, "-")) return new_node(ND_NEG, unary(rest, tok->next), NULL);
	return primary(rest, tok);
}

// mul = unary ("*" unary | "/" unary)*
static node_t *
mul(token_t **rest, token_t *tok) {
	node_t *node = unary(&tok, tok);

	for (;;) {
		if (equal(tok, "*")) {
			node = new_node(ND_MUL, node, unary(&tok, tok->next));
			continue;
		}
		if (equal(tok, "/")) {
			node = new_node(ND_DIV, node, unary(&tok, tok->next));
			continue;
		}
		*rest = tok;
		return node;
	}
}

// add = mul ("+" mul | "-" mul)*
static node_t *
add(token_t **rest, token_t *tok) {
	node_t *node = mul(&tok, tok);

	for (;;) {
		if (equal(tok, "+")) {
			node = new_node(ND_ADD, node, mul(&tok, tok->next));
			continue;
		}
		if (equal(tok, "-")) {
			node = new_node(ND_SUB, node, mul(&tok, tok->next));
			continue;
		}
		*rest = tok;
		return node;
	}
}

// relational = add ("<" add | "<=" add | ">" add | ">=" add)*
static node_t *
relational(token_t **rest, token_t *tok) {
	node_t *node = add(&tok, tok);

	for (;;) {
		if (equal(tok, "<")) {
			node = new_node(ND_LT, node, add(&tok, tok->next));
			continue;
		}
		if (equal(tok, "<=")) {
			node = new_node(ND_LE, node, add(&tok, tok->next));
			continue;
		}
		if (equal(tok, ">")) {
			node = new_node(ND_LT, add(&tok, tok->next), node);
			continue;
		}
		if (equal(tok, ">=")) {
			node = new_node(ND_LE, add(&tok, tok->next), node);
			continue;
		}
		*rest = tok;
		return node;
	}
}

// equality = relational ("==" relational | "!=" relational)*
static node_t *
equality(token_t **rest, token_t *tok) {
	node_t *node = relational(&tok, tok);

	for (;;) {
		if (equal(tok, "==")) {
			node = new_node(ND_EQ, node, relational(&tok, tok->next));
			continue;
		}
		if (equal(tok, "!=")) {
			node = new_node(ND_NE, node, relational(&tok, tok->next));
			continue;
		}
		*rest = tok;
		return node;
	}
}

// expr = equality
static node_t *
expr(token_t **rest, token_t *tok) {
	return equality(rest, tok);
}

// parse = expr
node_t *
parse(token_t *tok) {
	node_t *node = expr(&tok, tok);
	//	if (tok->type != TK_EOF) error_tok(tok, "extra token");
	return node;
}
