#include "mycc.h"

// Memory management policy.
// Do not release allocated memory.
// All allocated memories are released at the end of the process.

int
main(int argc, char **argv) {
	if (argc != 2) error("The number of arguments is incorrect.\n");

	// Tokenize and parse.
	token_t *token = tokenize(argv[1]);
	node_t *node   = parse(token);
	code_gen(node);

	return 0;
}
