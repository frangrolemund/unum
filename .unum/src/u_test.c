/*---------------------------------------------------------------
| vi: set noet ts=4 sw=4 fenc=utf-8
| ---------------------------------------------------------------
| Copyright 2024 Francis Henry Grolemund III
|
| Permission to use, copy, modify, and/or distribute this software for
| any purpose with or without fee is hereby granted, provided that the
| above copyright notice and this permission notice appear in all copies.
|
| THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL
| WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED
| WARRANTIES OF MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE
| AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL
| DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR
| PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER
| TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
| PERFORMANCE OF THIS SOFTWARE.
| ---------------------------------------------------------------*/

#include <stdlib.h>
#include "u_fs.h"
#include "u_test.h"

/*
 *  Unit-testing facilities:
 *  - simplified access to supporting test files near source
 *  - sandboxing of test files
 *  - cleanup of test
 *  - simple assert-like API.
 */
 
static char prog[U_PATH_MAX];

int _UT_run(const char *file, int argc, char *argv[],
	        UT_test_entry_t entry_fn) {
	int ret;
	
	UT_assert(argc > 0 && argv[0], "command-line not provided");
	ret = UU_basename(prog, argv[0], U_PATH_MAX);
		
	printf("TODO: INSIDE UT_TEST from %s\n", file);
	ret = entry_fn(argc, argv);
	printf("TODO: DEALLOCATING...\n");
	
	return ret;
}

void _UT_assert_failed(const char *expr, const char *file, int line,
	                   const char *msg) {
	fprintf(stderr, "%s: !! test failure !!\n", prog);
	fprintf(stderr, "%s: %s <-- '%s'\n", prog, msg, expr);
	fprintf(stderr, "%s: %s@%d\n", prog, file, line);
	exit(1);
}
