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

#include <stdarg.h>
#include <stdlib.h>
#include <string.h>

#include "u_fs.h"
#include "u_test.h"

/*
 *  Unit-testing facilities:
 *  - simplified access to supporting test files near source
 *  - sandboxing of test files
 *  - cleanup of test
 *  - simple assert-like API.
 */
 
static char prog[256];
static char src_path[U_PATH_MAX];
static const char *test_name      = NULL;


void _UT_test_failed( const char *expr, const char *file, int line,
				      const char *msg ) {
	fprintf(stderr, "%s: !! test failure !!\n", prog);
	fprintf(stderr, "%s: %s <-- '%s'\n", prog, msg, expr);
	fprintf(stderr, "%s: %s@%d\n", prog, file, line);
	exit(1);
}


void UT_printf( const char *fmt, ... ) {
	char     buf[2048];
	va_list  val;

	va_start(val, fmt);
	vsnprintf(buf, sizeof(buf), fmt, val);
	va_end(val);
	
	if (test_name) {
		fprintf(stdout, "%s (%s): %s\n", prog, test_name, buf);
	} else {
		fprintf(stdout, "%s: %s\n", prog, buf);
	}
}


char *UT_read_rel_path( const char *file ) {
	static char ret[U_PATH_MAX];

	UT_test_assert(file && *file, "File invalid.");
	
	strcpy(ret, src_path);
	strcat(ret, file);
	
	UT_test_assert(UU_is_file(ret), "File not found.");
		
	return ret;
}


int _UT_test( const char *file, int argc, char *argv[],
	         UT_test_entry_t entry_fn ) {
	int ret;
	
	UT_test_assert(argc > 0 && argv[0], "command-line not provided");

	ret = UU_basename(prog, argv[0], sizeof(prog));
	UT_test_assert(ret == UU_OK, "invalid program");
	
	ret = UU_dirname(src_path, file, sizeof(src_path));
	UT_test_assert(ret == UU_OK, "invalid source file");
		
	printf("TODO: INSIDE UT_TEST from %s\n", src_path);
	ret = entry_fn(argc, argv);
	printf("TODO: DEALLOCATING...\n");
	
	return ret;
}


void UT_set_test_name( const char *name ) {
	test_name = name;
}
