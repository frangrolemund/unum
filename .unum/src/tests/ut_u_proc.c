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

#include "u_test.h"

static uu_cstring_t prog;

static int unittest_proc( int argc, char *argv[] );
static int selftest_run( uu_cstring_t arg_selftest );

#define ARG_SELFTEST "--selftest="

int main( int argc, char *argv[] ) {
	uu_cstring_t a_selftest = NULL;
	size_t       len_st     = strlen(ARG_SELFTEST);
	for (int i = 0; i < argc; i++) {
		if (!strncmp(argv[i], ARG_SELFTEST, len_st)) {
			a_selftest = argv[i] + len_st;
			break;
		}
	}
	
	// - this test is its own child-process
	if (a_selftest) {
		return selftest_run(a_selftest);
	
	} else {
		prog = argv[0];
		return UT_test_run(argc, argv, unittest_proc);
	}
}


static int unittest_proc( int argc, char *argv[] ) {
	UT_test_printf("TODO: write test!");
	
	return 0;
}


static int selftest_run( uu_cstring_t arg_selftest ) {
	if (!strcmp(arg_selftest, "okrc")) {
		fprintf(stdout, "ut_u_proc: success\n");
		return 0;
		
	} else if (!strcmp(arg_selftest, "badrc")) {
		fprintf(stderr, "ut_u_proc: planned fail\n");
		return 3;
	}

	fprintf(stderr, "ut_u_proc: unsupported self-test '%s'\n", arg_selftest);
	return -255;
}
