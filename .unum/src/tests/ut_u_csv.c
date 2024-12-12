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
#include "u_csv.h"

static int unittest_csv( int argc, char *argv[] );
static void csv_test_simple( void );


int main( int argc, char *argv[] ) {
	return UT_test(argc, argv, unittest_csv);
}


static int unittest_csv( int argc, char *argv[] ) {
	csv_test_simple();
	
	return 0;
}


static void csv_assert_value(uu_csv_t *csv, unsigned row, unsigned col,
							 const char *value) {
	const char *tmp = NULL;
	uu_error_e err  = UU_OK;
	
	UU_assert(csv);
	
	tmp = UU_csv_get(csv, row, col, &err);
	UU_assert(err == UU_OK);
	
	UU_assert((tmp && value && !strcmp(tmp, value)) || (!tmp && !value));
}


static void csv_test_simple( void ) {
	uu_csv_t *cf;

	UT_set_test_name("simple, contrived parsing");
	
	// - eol variations
	UT_printf("eol testing...");
	cf = UU_csv_memory("aaa,bbb,ccc\r\n"
	                   "ddd,eee,fff\n"
	                   "ggg,hhh,iii\r\n"
	                   "jjj,kkk,lll",     NULL);
	UT_test_assert(cf != NULL, "failed to parse memory buffer.");
	
	UT_test_assert(UU_csv_cols(cf) == 3, "Failed to identify columns.");
	UT_test_assert(UU_csv_rows(cf) == 4, "Failed to identify rows.");
	csv_assert_value(cf, 0, 1, "bbb");
	csv_assert_value(cf, 1, 2, "fff");
	csv_assert_value(cf, 2, 0, "ggg");
	csv_assert_value(cf, 3, 2, "lll");
	
	UU_csv_delete(cf);
	
	// - field presence
	UT_printf("field presence testing...");
	cf = UU_csv_memory("000,,111\n"
	                   ",222,333\n"
	                   "444,555,\n", NULL);
	UT_test_assert(cf != NULL, "failed to parse memory buffer.");
	UT_test_assert(UU_csv_cols(cf) == 3, "Failed to identify columns.");
	UT_test_assert(UU_csv_rows(cf) == 3, "Failed to identify rows.");
	csv_assert_value(cf, 0, 0, "000");
	csv_assert_value(cf, 0, 1, NULL);
	csv_assert_value(cf, 0, 2, "111");
	csv_assert_value(cf, 1, 0, NULL);
	csv_assert_value(cf, 1, 1, "222");
	csv_assert_value(cf, 1, 2, "333");
	csv_assert_value(cf, 2, 0, "444");
	csv_assert_value(cf, 2, 1, "555");
	csv_assert_value(cf, 2, 2, NULL);
}
