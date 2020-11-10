#include "mycc.h"

// Stack operation
static void
gen_expr(node_t *node) {
	if (node->type == ND_NUM) {
		printf("  mov rax, %d\n", node->val);
		return;
	}
	if (node->type == ND_NEG) {
		gen_expr(node->lhs);
		printf("  neg rax\n");
		return;
	}

	gen_expr(node->rhs);
	printf("  push rax\n");
	gen_expr(node->lhs);
	printf("  pop rdi\n");

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
		case ND_EQ:
		case ND_NE:
		case ND_LT:
		case ND_LE:
			printf("  cmp rax, rdi\n");
			if (node->type == ND_EQ) printf("  sete al\n");
			if (node->type == ND_NE) printf("  setne al\n");
			if (node->type == ND_LT) printf("  setl al\n");
			if (node->type == ND_LE) printf("  setle al\n");
			printf("  movzb rax, al\n");
			break;
	}
}

void
code_gen(node_t *node) {
	// Outputs the first part of the assembly.
	printf(".intel_syntax noprefix\n");
	printf(".globl main\n");
	printf("main:\n");

	// Generate code from AST
	gen_expr(node);

	printf("  ret\n");
}
