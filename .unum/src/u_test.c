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
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#include "u_fs.h"
#include "u_test.h"
#include "u_mem.h"

/*
 *  Unit-testing facilities:
 *  - simplified access to supporting test files near source
 *  - sandboxing of test files
 *  - cleanup of test
 *  - simple assert-like API.
 */
 
static char        prog[256];
static uu_path_t   src_path;
static const char  *test_name = NULL;
static uu_string_t tmp_dir    = NULL;
static uu_string_t *tmp_files = NULL;
static int         num_tmp    = 0;

static void  discard_tmp_data( void );


int _UT_test( uu_cstring_t file, int argc, uu_string_t argv[],
	         UT_test_entry_t entry_fn ) {
	int ret;
	
	UT_assert(argc > 0 && argv[0], "command-line not provided");

	ret = UU_basename(prog, sizeof(prog), argv[0]);
	UT_assert(ret == UU_OK, "invalid program");
	
	ret = UU_dirname(src_path, sizeof(src_path), file);
	UT_assert(ret == UU_OK, "invalid source file");
	
	UT_set_name("--- BEGIN");
	UT_printf("unit test starting");
	test_name = NULL;
	
	ret = entry_fn(argc, argv);
	
	UT_set_name("-- RESULT");
	
	discard_tmp_data();
	
	if (UU_memc_num_bytes()) {
		UT_printf("memory leaks detected");
		UT_assert(0 == UU_memc_dump(), "memory leak(s) detected")
	}
	
	if (ret) {
		UT_printf("unit test failed with return code %d", ret);
	} else {
		UT_printf("unit test OK");
	}
	
	return ret;
}


void _UT_test_failed( uu_cstring_t expr, uu_cstring_t file, int line,
                      uu_cstring_t msg ) {
	char  prefix[256] = {"\0"};
	
	if (test_name) {
		sprintf(prefix, "%s (%s)", prog, test_name);
	} else {
		strcpy(prefix, prog);
	}
                      
	fprintf(stderr, "%s: !! TEST FAILURE !!\n", prefix);
	fprintf(stderr, "%s: %s <-- '%s'\n", prefix, msg, expr);
	fprintf(stderr, "%s: %s@%d\n", prefix, file, line);
	assert(0);
	exit(1);
}


void UT_set_name( uu_cstring_t name ) {
	if (test_name != NULL) {
		UT_printf("OK");
	}
	test_name = name;
}


void UT_printf( uu_cstring_t fmt, ... ) {
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


char *UT_rel_file( uu_cstring_t file ) {
	static uu_path_t ret;

	UT_assert(file && *file, "File invalid.");
	
	strcpy(ret, src_path);
	strcat(ret, file);
	
	UT_assert(UU_is_file(ret), "File not found.");
		
	return ret;
}


uu_cstring_t UT_tmpnam( void ) {
	static uu_path_t ret;
	time_t           now;
	struct tm        *tm_now;
	char             buf[32];
	uu_string_t      tf;
	
	now     = time(NULL);

	if (!tmp_dir) {
		tm_now  = localtime(&now);
		snprintf(buf, sizeof(buf)/sizeof(buf[0]),
                 "%02d%02d%02d-%02d%02d%02d", tm_now->tm_mon + 1,
                 tm_now->tm_mday, tm_now->tm_year - 100, tm_now->tm_hour,
				 tm_now->tm_min, tm_now->tm_sec);
		tmp_dir = (uu_string_t) UU_path_join_s(UNUM_DIR_TEST, prog, buf,
		                                       NULL);
		UT_assert(tmp_dir && (tmp_dir = UU_strdup(tmp_dir)), "temp failure");
		UU_mem_tare(tmp_dir);
		UT_assert(UU_mkdir(tmp_dir, S_IRWXU, true) == UU_OK, "mkdir failure");
	}
	
	strcpy(ret, tmp_dir);
	snprintf(buf, sizeof(buf)/sizeof(buf[0]), "%ld%d.tmp", now, num_tmp);
	strcat(ret, buf);
	
	num_tmp++;
	tmp_files = UU_realloc(tmp_files, sizeof(uu_string_t) * num_tmp);
	UT_assert(tmp_files, "out of memory");
	UU_mem_tare(tmp_files);
	tmp_files[num_tmp - 1] = tf = UU_strdup(ret);
	UT_assert(tf, "out of memory");
	UU_mem_tare(tf);
	
	return ret;
}


static void discard_tmp_data( void ) {
	uu_bool_t ok = true;
	
	if (!tmp_dir) {
		return;
	}
	
	for (int i = 0; i < num_tmp; i++) {
		if (UU_is_file(tmp_files[i]) && unlink(tmp_files[i]) != 0) {
			UT_printf("error: failed to delete %s (errno=%d)", tmp_files[i],
			          errno);
			ok = false;
		}
	}
	
	UT_assert(ok, "failed to delete temporary files.");
	
	if (UU_is_dir(tmp_dir) && rmdir(tmp_dir) != 0) {
		UT_printf("warning: failed to remove temporary directory %s", tmp_dir);
	}
}
