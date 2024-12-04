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

static int unittest_csv( int argc, char *argv[] );
static void csv_test_basic_parse( void );


int main( int argc, char *argv[] ) {
	return UT_test(argc, argv, unittest_csv);
}


static int unittest_csv( int argc, char *argv[] ) {
	csv_test_basic_parse();
	// - test-2: generate csv
	return 0;
}


static void csv_test_basic_parse( void ) {
	UT_set_test_name("basic parsing");
	UT_printf("reading test data...");
	UT_printf("...from file %s", UT_read_rel_path("ut_u_csv_1.csv"));
	// - test-1: get full filename of test file
	// - parse test file
	// - assert contents.
}
