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
static void csv_test_simple_file_1( void );
static uu_csv_t *read_test_file( uu_cstring_t file );
static void csv_test_simple_file_2( void );
static void csv_test_simple_mod_1( void );


int main( int argc, char *argv[] ) {
	return UT_test(argc, argv, unittest_csv);
}


static int unittest_csv( int argc, char *argv[] ) {
	csv_test_simple();
	csv_test_simple_file_1();
	csv_test_simple_file_2();
	csv_test_simple_mod_1();
	
	return 0;
}


static void csv_assert_value(uu_csv_t *csv, unsigned row, unsigned col,
							 const char *value) {
	const char *tmp = NULL;
	uu_error_e err  = UU_OK;
	
	tmp = UU_csv_get(csv, row, col, &err);
	UT_assert(err == UU_OK, "unexpected value");
	
	UT_assert((tmp && value && !strcmp(tmp, value)) || (!tmp && !value),
			  "unexpected value");
}


static void csv_test_simple( void ) {
	uu_csv_t *cf;

	UT_set_name("contrived parsing");
	
	// - eol variations
	UT_printf("eol testing...");
	cf = UU_csv_memory("aaa,bbb,ccc\r\n"
	                   "ddd,eee,fff\n"
	                   "ggg,hhh,iii\r\n"
	                   "jjj,kkk,lll",     NULL);
	UT_assert(cf, "failed to parse memory buffer.");
	
	UT_assert(UU_csv_cols(cf) == 3, "Failed to identify columns.");
	UT_assert(UU_csv_rows(cf) == 4, "Failed to identify rows.");
	csv_assert_value(cf, 0, 1, "bbb");
	csv_assert_value(cf, 1, 2, "fff");
	csv_assert_value(cf, 2, 0, "ggg");
	csv_assert_value(cf, 3, 2, "lll");
	
	UT_assert(!UU_csv_file_path(cf), "invalid file path");
	
	UU_csv_delete(cf);
	
	// - field presence
	UT_printf("field presence testing...");
	cf = UU_csv_memory("000,,111\n"
	                   ",222,333\n"
	                   "444,555,\n", NULL);
	UT_assert(cf, "failed to parse memory buffer.");
	UT_assert(UU_csv_cols(cf) == 3, "Failed to identify columns.");
	UT_assert(UU_csv_rows(cf) == 3, "Failed to identify rows.");
	csv_assert_value(cf, 0, 0, "000");
	csv_assert_value(cf, 0, 1, NULL);
	csv_assert_value(cf, 0, 2, "111");
	csv_assert_value(cf, 1, 0, NULL);
	csv_assert_value(cf, 1, 1, "222");
	csv_assert_value(cf, 1, 2, "333");
	csv_assert_value(cf, 2, 0, "444");
	csv_assert_value(cf, 2, 1, "555");
	csv_assert_value(cf, 2, 2, NULL);
	
	UU_csv_delete(cf);
	
	// - quotes
	UT_printf("quote testing...");
	cf = UU_csv_memory("aaa,bbb,ccc,111\n"
	                   "\"ddd\",\"eee\",\"ff,f\",2222\n"
	                   "ggg,\"hhh\r\nhh\",iii,33333\n", NULL);
	UT_assert(cf, "failed to parse memory buffer.");
	UT_assert(UU_csv_cols(cf) == 4, "Failed to identify columns.");
	UT_assert(UU_csv_rows(cf) == 3, "Failed to identify rows.");
	csv_assert_value(cf, 0, 0, "aaa");
	csv_assert_value(cf, 1, 0, "ddd");
	csv_assert_value(cf, 1, 1, "eee");
	csv_assert_value(cf, 1, 2, "ff,f");
	csv_assert_value(cf, 1, 3, "2222");
	csv_assert_value(cf, 2, 0, "ggg");
	csv_assert_value(cf, 2, 1, "hhh\r\nhh");
	csv_assert_value(cf, 2, 2, "iii");
	csv_assert_value(cf, 2, 3, "33333");
	
	UU_csv_delete(cf);
	
	UT_printf("quote escaping testing...");
	cf = UU_csv_memory("aaa,bb\"b,ccc\n"
	                   "\"ddd\",\"eee\"\",ee\"\"ee\",\"fff\"", NULL);
	UT_assert(cf, "failed to parse memory buffer.");
	UT_assert(UU_csv_cols(cf) == 3, "Failed to identify columns.");
	UT_assert(UU_csv_rows(cf) == 2, "Failed to identify rows.");
	csv_assert_value(cf, 0, 0, "aaa");
	csv_assert_value(cf, 0, 1, "bb\"b");
	csv_assert_value(cf, 0, 2, "ccc");
	csv_assert_value(cf, 1, 0, "ddd");
	csv_assert_value(cf, 1, 1, "eee\",ee\"ee");
	csv_assert_value(cf, 1, 2, "fff");
	
	UU_csv_delete(cf);

}


static void csv_test_simple_file_1( void ) {
	uu_csv_t *cf;

	UT_set_name("file parsing #1");

	cf = read_test_file("ut_u_csv_1.csv");
	
	UT_printf("verifying file structure");
	UT_assert(UU_csv_cols(cf) == 5, "Failed to identify columns.");
	UT_assert(UU_csv_rows(cf) == 5, "Failed to identify rows.");

	csv_assert_value(cf, 0, 0, "u_bool");
	csv_assert_value(cf, 0, 1, "u_path");
	csv_assert_value(cf, 0, 2, "u_desc");
	csv_assert_value(cf, 0, 3, "u_text_ml");
	csv_assert_value(cf, 0, 4, "u_num");
	
	csv_assert_value(cf, 1, 0, "FALSE");
	csv_assert_value(cf, 1, 1, "/usr/bin/pgrep");
	csv_assert_value(cf, 1, 2, "search process table");
	csv_assert_value(cf, 1, 3, "The \"pgrep\" command searches "
							   "the process table \n\non the running system "
	                           "and prints the process \n\nIDs of all "
	                           "processes that match the criteria \n\n"
	                           "given on the command line.");
	csv_assert_value(cf, 1, 4, NULL);

	csv_assert_value(cf, 2, 0, "TRUE");
	csv_assert_value(cf, 2, 1, "/usr/sbin/chroot");
	csv_assert_value(cf, 2, 2, "change root directory");
	csv_assert_value(cf, 2, 3, NULL);
	csv_assert_value(cf, 2, 4, "-4");
	
	csv_assert_value(cf, 3, 0, "TRUE");
	csv_assert_value(cf, 3, 1, "/etc/passwd");
	csv_assert_value(cf, 3, 2, NULL);
	csv_assert_value(cf, 3, 3, "User Database\n\n"
	                           "Note that this file is consulted directly "
	                           "only when the system is running\n\n"
	                           "in single-user mode.  At other times this "
	                           "information is provided by\n\nOpen Directory.");
	csv_assert_value(cf, 3, 4, "100.2");
	
	csv_assert_value(cf, 4, 0, NULL);
	csv_assert_value(cf, 4, 1, "/bin/sleep");
	csv_assert_value(cf, 4, 2, "delay");
	csv_assert_value(cf, 4, 3, "The \"sleep\" command suspends execution for "
							   "a minimum of seconds.\n\n"
	                           "If the sleep command receives a signal, it "
							   "takes the standard action.\n\n"
	                           "When the SIGINFO signal is received, the "
	                           "estimate of the amount of\n\n"
	                           "seconds left to sleep is printed on the "
	                           "standard output.");
	csv_assert_value(cf, 4, 4, "8");
	
	UT_assert(UU_csv_file_path(cf), "invalid file path");
	
	UU_csv_delete(cf);
}


static uu_csv_t *read_test_file( uu_cstring_t file ) {
	uu_csv_t *cf;
	
	UT_printf("reading %s", file);
	cf = UU_csv_open(UT_rel_file(file), NULL);
	UT_assert(cf, "failed to read test file.");
	
	return cf;
}


static void csv_test_simple_file_2( void ) {
	uu_csv_t *cf;
	int      i, j;

	UT_set_name("file parsing #2");

	cf = read_test_file("ut_u_csv_2.csv");
	
	UT_printf("verifying file structure");
	UT_assert(UU_csv_cols(cf) == 12, "Failed to identify columns.");
	UT_assert(UU_csv_rows(cf) == 10001, "Failed to identify rows.");
	
	for (i = 0; i < UU_csv_rows(cf); i++) {
		for (j = 0; j < UU_csv_cols(cf); j++) {
			UT_assert(UU_csv_get(cf, i, j, NULL), "Failed to find data.");
		}
	}

	UU_csv_delete(cf);
}


static void csv_test_simple_mod_1( void ) {
	uu_csv_t *cf;

	UT_set_name("file modification #1");
	
	cf = read_test_file("ut_u_csv_1.csv");

	UT_printf("modifying and verifying");
	UT_assert(UU_csv_set(cf, 0, 2, "stars") == UU_OK, "failed to assign value");
	UT_assert(UU_csv_set(cf, 2, 3, "launch") == UU_OK,
	          "failed to assign value");
	UT_assert(UU_csv_set(cf, 4, 4, "orbit") == UU_OK, "failed to assign value");
	
	UT_assert_eq(UU_csv_get(cf, 0, 2, NULL), "stars", "failed to find value");

	UT_assert_eq(UU_csv_get(cf, 2, 3, NULL), "launch", "failed to find value");

	UT_assert_eq(UU_csv_get(cf, 4, 4, NULL), "orbit", "failed to find value");

	UT_assert(UU_csv_set(cf, -1, 4, "rover") == UU_ERR_ARGS,
	                     "failed to detect error");
	                     
	UT_assert(UU_csv_set(cf, 6, 4, "chute") == UU_ERR_ARGS,
	                     "failed to detect error");
	                     
	UT_assert(UU_csv_set(cf, 3, 6, "payload") == UU_ERR_ARGS,
	                     "failed to detect error");
			  
	UU_csv_delete(cf);
}
