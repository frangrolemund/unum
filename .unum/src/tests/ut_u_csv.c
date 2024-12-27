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
static void assert_mem_file( uu_csv_t *csv, void (*test)(uu_csv_t *csv));
static void csv_sm1_verify1( uu_csv_t *csv );
static void csv_test_simple_mod_2( void );
static void csv_sm2_del_verify1( uu_csv_t *csv );
static void csv_sm2_del_verify2( uu_csv_t *csv );
static void csv_sm2_add_verify3( uu_csv_t *csv );
static void csv_sm2_add_verify4( uu_csv_t *csv );
static void csv_sm2_ins_verify5( uu_csv_t *csv );
static void csv_sm2_ins_verify6( uu_csv_t *csv );


int main( int argc, char *argv[] ) {
	return UT_test_run(argc, argv, unittest_csv);
}


static int unittest_csv( int argc, char *argv[] ) {
	csv_test_simple();
	csv_test_simple_file_1();
	csv_test_simple_file_2();
	csv_test_simple_mod_1();
	csv_test_simple_mod_2();
	
	return 0;
}


static void csv_assert_value(uu_csv_t *csv, unsigned row, unsigned col,
							 const char *value) {
	const char *tmp = NULL;
	uu_error_e err  = UU_OK;
	
	tmp = UU_csv_get(csv, row, col, &err);
	UT_test_assert(err == UU_OK, "unexpected value");
	
	UT_test_assert((tmp && value && !strcmp(tmp, value)) || (!tmp && !value),
			  "unexpected value");
}


static void csv_test_simple( void ) {
	uu_csv_t *cf;

	UT_test_setname("contrived parsing");
	
	// - eol variations
	UT_test_printf("eol testing...");
	cf = UU_csv_memory("aaa,bbb,ccc\r\n"
	                   "ddd,eee,fff\n"
	                   "ggg,hhh,iii\r\n"
	                   "jjj,kkk,lll",     NULL);
	UT_test_assert(cf, "failed to parse memory buffer.");
	
	UT_test_assert(UU_csv_col_count(cf) == 3, "Failed to identify columns.");
	UT_test_assert(UU_csv_row_count(cf) == 4, "Failed to identify rows.");
	csv_assert_value(cf, 0, 1, "bbb");
	csv_assert_value(cf, 1, 2, "fff");
	csv_assert_value(cf, 2, 0, "ggg");
	csv_assert_value(cf, 3, 2, "lll");
	
	UT_test_assert(!UU_csv_file_path(cf), "invalid file path");
	
	UU_csv_delete(cf);
	
	// - field presence
	UT_test_printf("field presence testing...");
	cf = UU_csv_memory("000,,111\n"
	                   ",222,333\n"
	                   "444,555,\n", NULL);
	UT_test_assert(cf, "failed to parse memory buffer.");
	UT_test_assert(UU_csv_col_count(cf) == 3, "Failed to identify columns.");
	UT_test_assert(UU_csv_row_count(cf) == 3, "Failed to identify rows.");
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
	UT_test_printf("quote testing...");
	cf = UU_csv_memory("aaa,bbb,ccc,111\n"
	                   "\"ddd\",\"eee\",\"ff,f\",2222\n"
	                   "ggg,\"hhh\r\nhh\",iii,33333\n", NULL);
	UT_test_assert(cf, "failed to parse memory buffer.");
	UT_test_assert(UU_csv_col_count(cf) == 4, "Failed to identify columns.");
	UT_test_assert(UU_csv_row_count(cf) == 3, "Failed to identify rows.");
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
	
	UT_test_printf("quote escaping testing...");
	cf = UU_csv_memory("aaa,bb\"b,ccc\n"
	                   "\"ddd\",\"eee\"\",ee\"\"ee\",\"fff\"", NULL);
	UT_test_assert(cf, "failed to parse memory buffer.");
	UT_test_assert(UU_csv_col_count(cf) == 3, "Failed to identify columns.");
	UT_test_assert(UU_csv_row_count(cf) == 2, "Failed to identify rows.");
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

	UT_test_setname("file parsing #1");

	cf = read_test_file("ut_u_csv_1.csv");
	
	UT_test_printf("verifying file structure");
	UT_test_assert(UU_csv_col_count(cf) == 5, "Failed to identify columns.");
	UT_test_assert(UU_csv_row_count(cf) == 5, "Failed to identify rows.");

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
	
	UT_test_assert(UU_csv_file_path(cf), "invalid file path");
	
	UU_csv_delete(cf);
}


static uu_csv_t *read_test_file( uu_cstring_t file ) {
	uu_csv_t *cf;
	
	UT_test_printf("reading %s", file);
	cf = UU_csv_open(UT_test_filename(file), NULL);
	UT_test_assert(cf, "failed to read test file.");
	
	return cf;
}


static void csv_test_simple_file_2( void ) {
	uu_csv_t *cf;
	int      i, j;

	UT_test_setname("file parsing #2");

	cf = read_test_file("ut_u_csv_2.csv");
	
	UT_test_printf("verifying file structure");
	UT_test_assert(UU_csv_col_count(cf) == 12, "Failed to identify columns.");
	UT_test_assert(UU_csv_row_count(cf) == 10001, "Failed to identify rows.");
	
	for (i = 0; i < UU_csv_row_count(cf); i++) {
		for (j = 0; j < UU_csv_col_count(cf); j++) {
			UT_test_assert(UU_csv_get(cf, i, j, NULL), "Failed to find data.");
		}
	}

	UU_csv_delete(cf);
}


static void csv_test_simple_mod_1( void ) {
	uu_csv_t     *cf;

	UT_test_setname("file mod #1");
	
	cf = read_test_file("ut_u_csv_1.csv");

	UT_test_printf("modifying and writing to file");
	UT_test_assert(UU_csv_set(cf, 0, 2, "stars") == UU_OK,
	               "failed to assign value");
	UT_test_assert(UU_csv_set(cf, 2, 3, "launch\ndate") == UU_OK,
	               "failed to assign value");
	UT_test_assert(UU_csv_set(cf, 4, 4, "orbit \"every\" day") == UU_OK,
	               "failed to assign value");
	               
	assert_mem_file(cf, csv_sm1_verify1);
			  
	UU_csv_delete(cf);
}


static void assert_mem_file( uu_csv_t *csv, void (*test)(uu_csv_t *csv)) {
	uu_cstring_t tmp_name;
	uu_csv_t     *cf;
	uu_error_e	 err      = UU_OK;
	
	UT_test_printf("...memory test");
	test(csv);
	
	UT_test_printf("...file test");
	tmp_name = UT_test_tempfile("csv");
	UT_test_assert(UU_csv_write(csv, tmp_name) == UU_OK, "failed to write");
	cf  = UU_csv_open(tmp_name, &err);
	UT_test_assert(cf, "failed to reopen");
	test(cf);
	UU_csv_delete(cf);
}


static void csv_sm1_verify1( uu_csv_t *csv ) {
	UT_test_assert_eq(UU_csv_get(csv, 0, 2, NULL), "stars",
	                  "failed to find value");

	UT_test_assert_eq(UU_csv_get(csv, 2, 3, NULL), "launch\ndate",
	                  "failed to find value");

	UT_test_assert_eq(UU_csv_get(csv, 4, 4, NULL), "orbit \"every\" day",
	                  "failed to find value");

	UT_test_assert(UU_csv_set(csv, -1, 4, "rover") == UU_ERR_ARGS,
				   "failed to detect error");
	                     
	UT_test_assert(UU_csv_set(csv, 6, 4, "chute") == UU_ERR_ARGS,
	               "failed to detect error");
	                     
	UT_test_assert(UU_csv_set(csv, 3, 6, "payload") == UU_ERR_ARGS,
	               "failed to detect error");
}


static void csv_test_simple_mod_2( void ) {
	uu_csv_t     *cf;

	UT_test_setname("file mod #2");
	
	cf = read_test_file("ut_u_csv_1.csv");
	UT_test_assert(cf && UU_csv_row_count(cf) == 5, "invalid CSV");
	
	UT_test_assert(UU_csv_delete_row(cf, 5) != UU_OK, "delete allowed?");
	
	UT_test_printf("deleting row 3");
	UT_test_assert(UU_csv_delete_row(cf, 3) == UU_OK, "delete failed");
	assert_mem_file(cf, csv_sm2_del_verify1);
	
	UT_test_printf("deleting row 0");
	UT_test_assert(UU_csv_delete_row(cf, 0) == UU_OK, "delete failed");
	assert_mem_file(cf, csv_sm2_del_verify2);
	
	UT_test_printf("adding row");
	UT_test_assert(UU_csv_add_row(cf) == 4, "add failed");
	assert_mem_file(cf, csv_sm2_add_verify3);
	assert_mem_file(cf, csv_sm2_del_verify2);
	
	UT_test_printf("modifying added row");
	UU_csv_set(cf, 3, 0, NULL);
	UU_csv_set(cf, 3, 1, "a");
	UU_csv_set(cf, 3, 2, NULL);
	UU_csv_set(cf, 3, 3, "b");
	UU_csv_set(cf, 3, 4, NULL);
	assert_mem_file(cf, csv_sm2_add_verify4);
	assert_mem_file(cf, csv_sm2_del_verify2);
	
	UT_test_printf("inserting row at 3");
	UT_test_assert(UU_csv_insert_row(cf, 3) == UU_OK, "insert failed");
	assert_mem_file(cf, csv_sm2_del_verify2);
	assert_mem_file(cf, csv_sm2_ins_verify5);
	
	UT_test_printf("modifying inserted row");
	UU_csv_set(cf, 3, 0, "c");
	UU_csv_set(cf, 3, 1, NULL);
	UU_csv_set(cf, 3, 2, "d");
	UU_csv_set(cf, 3, 3, NULL);
	UU_csv_set(cf, 3, 4, "e");
	assert_mem_file(cf, csv_sm2_ins_verify6);
	
	UU_csv_delete(cf);
}


static void csv_sm2_del_verify1( uu_csv_t *csv ) {
	csv_assert_value(csv, 0, 0, "u_bool");
	csv_assert_value(csv, 0, 1, "u_path");
	csv_assert_value(csv, 0, 2, "u_desc");
	csv_assert_value(csv, 0, 3, "u_text_ml");
	csv_assert_value(csv, 0, 4, "u_num");
	
	csv_assert_value(csv, 1, 0, "FALSE");
	csv_assert_value(csv, 1, 1, "/usr/bin/pgrep");
	csv_assert_value(csv, 1, 2, "search process table");
	csv_assert_value(csv, 1, 3, "The \"pgrep\" command searches "
						 	    "the process table \n\non the running system "
	                            "and prints the process \n\nIDs of all "
	                            "processes that match the criteria \n\n"
	                            "given on the command line.");
	csv_assert_value(csv, 1, 4, NULL);

	csv_assert_value(csv, 2, 0, "TRUE");
	csv_assert_value(csv, 2, 1, "/usr/sbin/chroot");
	csv_assert_value(csv, 2, 2, "change root directory");
	csv_assert_value(csv, 2, 3, NULL);
	csv_assert_value(csv, 2, 4, "-4");
		
	csv_assert_value(csv, 3, 0, NULL);
	csv_assert_value(csv, 3, 1, "/bin/sleep");
	csv_assert_value(csv, 3, 2, "delay");
	csv_assert_value(csv, 3, 3, "The \"sleep\" command suspends execution for "
							    "a minimum of seconds.\n\n"
	                            "If the sleep command receives a signal, it "
							    "takes the standard action.\n\n"
	                            "When the SIGINFO signal is received, the "
	                            "estimate of the amount of\n\n"
	                            "seconds left to sleep is printed on the "
	                            "standard output.");
	csv_assert_value(csv, 3, 4, "8");
}


static void csv_sm2_del_verify2( uu_csv_t *csv ) {
	csv_assert_value(csv, 0, 0, "FALSE");
	csv_assert_value(csv, 0, 1, "/usr/bin/pgrep");
	csv_assert_value(csv, 0, 2, "search process table");
	csv_assert_value(csv, 0, 3, "The \"pgrep\" command searches "
						 	    "the process table \n\non the running system "
	                            "and prints the process \n\nIDs of all "
	                            "processes that match the criteria \n\n"
	                            "given on the command line.");
	csv_assert_value(csv, 0, 4, NULL);

	csv_assert_value(csv, 1, 0, "TRUE");
	csv_assert_value(csv, 1, 1, "/usr/sbin/chroot");
	csv_assert_value(csv, 1, 2, "change root directory");
	csv_assert_value(csv, 1, 3, NULL);
	csv_assert_value(csv, 1, 4, "-4");
		
	csv_assert_value(csv, 2, 0, NULL);
	csv_assert_value(csv, 2, 1, "/bin/sleep");
	csv_assert_value(csv, 2, 2, "delay");
	csv_assert_value(csv, 2, 3, "The \"sleep\" command suspends execution for "
							    "a minimum of seconds.\n\n"
	                            "If the sleep command receives a signal, it "
							    "takes the standard action.\n\n"
	                            "When the SIGINFO signal is received, the "
	                            "estimate of the amount of\n\n"
	                            "seconds left to sleep is printed on the "
	                            "standard output.");
	csv_assert_value(csv, 2, 4, "8");
}


static void csv_sm2_add_verify3( uu_csv_t *csv ) {
	UT_test_assert(UU_csv_row_count(csv) == 4, "invalid row count");

	for (int i = 0; i < UU_csv_col_count(csv); i++) {
		csv_assert_value(csv, 3, 0, NULL);
	}
}


static void csv_sm2_add_verify4( uu_csv_t *csv ) {
	csv_assert_value(csv, 3, 0, NULL);
	csv_assert_value(csv, 3, 1, "a");
	csv_assert_value(csv, 3, 2, NULL);
	csv_assert_value(csv, 3, 3, "b");
	csv_assert_value(csv, 3, 4, NULL);
}


static void csv_sm2_ins_verify5( uu_csv_t *csv ) {
	UT_test_assert(UU_csv_row_count(csv) == 5, "invalid row count");

	for (int i = 0; i < UU_csv_col_count(csv); i++) {
		csv_assert_value(csv, 3, 0, NULL);
	}
}


static void csv_sm2_ins_verify6( uu_csv_t *csv ) {
	csv_assert_value(csv, 3, 0, "c");
	csv_assert_value(csv, 3, 1, NULL);
	csv_assert_value(csv, 3, 2, "d");
	csv_assert_value(csv, 3, 3, NULL);
	csv_assert_value(csv, 3, 4, "e");
	
	csv_assert_value(csv, 4, 0, NULL);
	csv_assert_value(csv, 4, 1, "a");
	csv_assert_value(csv, 4, 2, NULL);
	csv_assert_value(csv, 4, 3, "b");
	csv_assert_value(csv, 4, 4, NULL);
}
