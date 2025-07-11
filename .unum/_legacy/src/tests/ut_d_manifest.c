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

#include <time.h>

#include "u_common.h"
#include "d_manifest.h"
#include "u_test.h"

static int          unittest_manifest( int argc, char *argv[] );
static void         manifest_test_simple( void );
static uu_cstring_t tmp_root ( void );
static uu_cstring_t tmp_file_wdir( uu_cstring_t ext,
                                   uu_cstring_t path_offsets[] );


int main( int argc, char *argv[] ) {
	return UT_test_run(argc, argv, unittest_manifest);
}


static int unittest_manifest( int argc, char *argv[] ) {
	manifest_test_simple();
	return 0;
}


static void manifest_test_simple( void ) {
	ud_manifest_t      *man;
	uu_error_e         err;
	uu_path_t          tpath;
	uu_cstring_t       root, tf1, tf2, tf3, man_file;
	uu_cstring_t       bad_root;
	ud_manifest_file_t file;
	
	UT_test_setname("simple manifest");
	
	// - create
	root = tmp_root();
	UT_test_printf("root: %s", root);
	
	bad_root = UU_path_join_s(root, "bar", NULL);
	man = UD_manifest_new(bad_root, &err);
	UT_test_assert(!man && err == UU_ERR_FILE, "failed to detect missing root");
	
	man = UD_manifest_new(root, &err);
	UT_test_assert(man && err == UU_OK, "failed to create manifest");
	
	UT_test_assert(UU_path_normalize(tpath, root, NULL), "invalid path");
	UT_test_assert_eq(tpath, UD_manifest_root(man), "invalid root");
	
	tf1 = tmp_file_wdir("c",
					    (uu_cstring_t []){".unum", "src", "core", NULL});
	                         
	UT_test_assert(UD_manifest_add_file(man, (ud_manifest_file_t) {
		tf1,
		UD_MANP_CORE,
		UD_MANP_KERN,
		NULL
	}) != UU_OK, "failed to detect invalid dep");
	
	UT_test_assert(UD_manifest_add_file(man, (ud_manifest_file_t) {
		__FILE__,
		UD_MANP_CORE,
		UD_MANP_KERN,
		NULL
	}) != UU_OK, "failed to detect invalid file");

	UT_test_assert(UD_manifest_add_file(man, (ud_manifest_file_t) {
		tf1,
		UD_MANP_KERN,
		UD_MANP_CORE,
		NULL
	}) == UU_OK, "failed to add file");
	UT_test_printf("file-1: %s", tf1);
	UT_test_assert(UD_manifest_file_count(man) == 1, "invalid file count");
	
	// ...should be NOOP
	UT_test_assert(UD_manifest_add_file(man, (ud_manifest_file_t) {
		tf1,
		UD_MANP_KERN,
		UD_MANP_CORE,
		NULL
	}) == UU_OK, "failed to add file");
	UT_test_assert(UD_manifest_file_count(man) == 1, "invalid file count");
	
	tf2 = tmp_file_wdir("un", (uu_cstring_t []){"src", "server", NULL});
	UT_test_assert(UD_manifest_add_file(man, (ud_manifest_file_t) {
		tf2,
		UD_MANP_CUSTOM,
		UD_MANP_KERN,
		NULL
	}) == UU_OK, "failed to add file");
	UT_test_printf("file-2: %s", tf2);
	UT_test_assert(UD_manifest_file_count(man) == 2, "invalid file count");
	
	tf3 = tmp_file_wdir("un", (uu_cstring_t []){"src", "db", "tests", NULL});
	UT_test_assert(UD_manifest_add_file(man, (ud_manifest_file_t) {
		tf3,
		UD_MANP_TEST,
		UD_MANP_CUSTOM,
		NULL
	}) != UU_OK, "failed to detect invalid test");
	
	UT_test_assert(UD_manifest_add_file(man, (ud_manifest_file_t) {
		tf3,
		UD_MANP_TEST,
		UD_MANP_CUSTOM,
		"sample-test"
	}) == UU_OK, "failed to add file");
	UT_test_printf("file-3: %s", tf3);
	UT_test_assert(UD_manifest_file_count(man) == 3, "invalid file count");

	
	man_file = UT_test_tempfile("csv", NULL);
	UT_test_assert(UD_manifest_write(man, man_file) == UU_OK, "failed write");
	UT_test_printf("manifest: %s", man_file);
	
	UD_manifest_delete(man);
	
	// - reload
	man = UD_manifest_open(bad_root, man_file, &err);
	UT_test_assert(!man && err == UU_ERR_FILE, "failed to detect bad root");
	man = UD_manifest_open(root, man_file, &err);
	UT_test_assert(man && err == UU_OK, "failed to open manifest");
	UT_test_assert(UD_manifest_file_count(man) == 3, "invalid file count");

	UT_test_assert(UD_manifest_get(man, 0, &file), "failed to get file");
	UT_test_assert_eq(file.path, UU_path_normalize_s(tf1, NULL),
	                  "failed to match filename");
	UT_test_assert(file.phase == UD_MANP_KERN, "file phase invalid");
	UT_test_assert(file.req == UD_MANP_CORE, "file req invalid");
	UT_test_assert_eq(file.name, NULL, "file name invalid");
	
	UT_test_assert(UD_manifest_get(man, 1, &file), "failed to get file");
	UT_test_assert_eq(file.path, UU_path_normalize_s(tf2, NULL),
	                  "failed to match filename");
	UT_test_assert(file.phase == UD_MANP_CUSTOM, "file phase invalid");
	UT_test_assert(file.req == UD_MANP_KERN, "file req invalid");
	UT_test_assert_eq(file.name, NULL, "file name invalid");
	
	UT_test_assert(UD_manifest_get(man, 2, &file), "failed to get file");
	UT_test_assert_eq(file.path, UU_path_normalize_s(tf3, NULL),
	                  "failed to match filename");
	UT_test_assert(file.phase == UD_MANP_TEST, "file phase invalid");
	UT_test_assert(file.req == UD_MANP_CUSTOM, "file req invalid");
	UT_test_assert_eq(file.name, "sample-test", "file name invalid");
	
	// - delete items/verify
	UT_test_assert(UD_manifest_delete_file(man, tf2) == UU_OK,
	               "failed to delete");
	UT_test_assert(UD_manifest_file_count(man) == 2, "invalid file count");
	UT_test_assert(UD_manifest_delete_file_n(man, 1) == UU_OK,
	               "failed to delete");
	UT_test_assert(UD_manifest_file_count(man) == 1, "invalid file count");
	UT_test_assert(UD_manifest_write(man, NULL) == UU_OK, "failed to write");
	UD_manifest_delete(man);
	
	man = UD_manifest_open(root, man_file, NULL);
	UT_test_assert(man, "failed to reopen")
	UT_test_assert(UD_manifest_file_count(man) == 1, "invalid file count");
	UT_test_assert(UD_manifest_get(man, 0, &file), "failed to get file");
	UT_test_assert_eq(file.path, UU_path_normalize_s(tf1, NULL),
	                  "failed to match filename");
	UT_test_assert(file.phase == UD_MANP_KERN, "file phase invalid");
	UT_test_assert(file.req == UD_MANP_CORE, "file req invalid");
	UT_test_assert_eq(file.name, NULL, "file name invalid");	
	UD_manifest_delete(man);
}


static uu_cstring_t tmp_file_wdir( uu_cstring_t ext,
                                   uu_cstring_t path_offsets[] ) {
	uu_cstring_t ret;
	FILE         *fp;
	time_t       now;
	
	ret = UT_test_tempfile(ext, path_offsets);
	
	fp  = fopen(ret, "w");
	UT_test_assert(fp, "failed to open file.");
	now = time(NULL);
	UT_test_assert(fprintf(fp, "sample file: %ld", now) > 0, "failed to write");
	fclose(fp);
	
	return ret;
}

static uu_cstring_t tmp_root ( void ) {
	uu_cstring_t tmp_file;
	uu_string_t  ret;
	
	tmp_file = UT_test_tempfile("csv", NULL);
	ret = UU_mem_tare(UU_mem_strdup(tmp_file));
	UT_test_assert(ret, "out of memory")
	UT_test_assert(UU_path_dirname(ret, strlen(ret) + 1, ret) == UU_OK,
	               "failed to get dir");
	return ret;
}
