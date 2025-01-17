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

 
static uu_string_t track_tmp_file( uu_cstring_t file );
static void        delete_tmp_files( void );
static int         tmp_dir_compare(const void *f1, const void *f2);


static uu_path_t   test_dir   = {'\0'};
static char        prog[256];
static uu_path_t   src_path;
static const char  *test_name = NULL;
static uu_string_t tmp_dir    = NULL;
static uu_string_t *tmp_files = NULL;
static int         num_tmp    = 0;
static uu_bool_t   is_struct  = false;


#define ARG_STRUCTURED "--unum-test-struct"


int _UT_test_run( uu_cstring_t file, int argc, uu_string_t argv[],
	              UT_test_entry_t entry_fn ) {
	int ret;
	
	UT_test_assert(argc > 0 && argv[0], "command-line not provided");
	
	for (int i = 1; i < argc; i++) {
		if (!strcmp(argv[i], ARG_STRUCTURED)) {
			is_struct = true;
		}
	}
	
	ret = UU_path_basename(prog, sizeof(prog), argv[0]);
	UT_test_assert(ret == UU_OK, "invalid program");
	
	ret = UU_path_dirname(src_path, sizeof(src_path), file);
	UT_test_assert(ret == UU_OK, "invalid source file");
	
	UT_test_setname("--- BEGIN");
	UT_test_printf("unit test starting");
	test_name = NULL;
	
	ret = entry_fn(argc, argv);
	
	UT_test_setname("-- RESULT");
	
	delete_tmp_files();
	
	if (UU_memc_num_bytes()) {
		UT_test_printf("memory leaks detected");
		UT_test_assert(0 == UU_memc_dump(), "memory leak(s) detected")
	}
	
	if (ret) {
		UT_test_printf("unit test failed with return code %d", ret);
	} else {
		UT_test_printf("unit test OK");
	}
	
	return ret;
}


void _UT_test_failed( uu_cstring_t expr, uu_cstring_t file, int line,
                      uu_cstring_t msg ) {
	char        prefix[256] = {"\0"};
	
	if (test_name) {
		sprintf(prefix, "%s (%s)", prog, test_name);
	} else {
		strcpy(prefix, prog);
	}
	
	if (is_struct) {
		fprintf(stderr, "<uerr file=\"%s\" line=\"%d\">%s <-- '%s'</uerr>\n",
		        file, line, msg, expr);
	
	} else {
		fprintf(stderr, "%s: !! TEST FAILURE !!\n", prefix);
		fprintf(stderr, "%s: %s <-- '%s'\n", prefix, msg, expr);
		fprintf(stderr, "%s: %s@%d\n", prefix, file, line);
		assert(0);
	}
	exit(1);
}


void UT_test_assert_eq( uu_cstring_t s1, uu_cstring_t s2, uu_cstring_t msg ) {
	if (!s1 && !s2) {
		return;
	}
	
	UT_test_assert(s1 && s2, msg);
	UT_test_assert(!strcmp((s1), (s2)), msg);
}


uu_cstring_t UT_test_dir( void ) {
	if (!test_dir[0]) {
		UU_path_join(test_dir, sizeof(test_dir), UNUM_DIR_CODE_BASIS,
		             UNUM_DIR_DEPLOY, "test", NULL);
	}
	return test_dir;
}


void UT_test_setname( uu_cstring_t name ) {
	if (test_name != NULL) {
		UT_test_printf("OK");
	}
	test_name = name;
}


void UT_test_printf( uu_cstring_t fmt, ... ) {
	char        buf[2048];
	va_list     val;

	va_start(val, fmt);
	vsnprintf(buf, sizeof(buf), fmt, val);
	va_end(val);
	
	if (is_struct) {
		if (test_name) {
			fprintf(stdout, "<uout>(%s): %s</uout>\n", test_name, buf);
		} else {
			fprintf(stdout, "<uout>%s</uout>\n", buf);
		}
	} else {
		if (test_name) {
			fprintf(stdout, "%s (%s): %s\n", prog, test_name, buf);
		} else {
			fprintf(stdout, "%s: %s\n", prog, buf);
		}
	}
}


char *UT_test_filename( uu_cstring_t file ) {
	static uu_path_t ret;

	UT_test_assert(file && *file, "File invalid.");
	
	strcpy(ret, src_path);
	strcat(ret, file);
	
	UT_test_assert(UU_file_exists(ret), "File not found.");
		
	return ret;
}


uu_cstring_t UT_test_tempfile( uu_cstring_t extension,
                               uu_cstring_t subdirs[] ) {
	time_t           now;
	struct tm        *tm_now;
	char             buf[32];
	uu_string_t      ret;
	
	now     = time(NULL);

	if (!tmp_dir) {
		tm_now  = localtime(&now);
		snprintf(buf, sizeof(buf)/sizeof(buf[0]),
                 "%02d%02d%02d-%02d%02d%02d", tm_now->tm_mon + 1,
                 tm_now->tm_mday, tm_now->tm_year - 100, tm_now->tm_hour,
				 tm_now->tm_min, tm_now->tm_sec);
		tmp_dir = (uu_string_t) UU_path_join_s(UT_test_dir(), prog, buf,
		                                       NULL);
		UT_test_assert(tmp_dir && (tmp_dir = UU_mem_strdup(tmp_dir)),
		               "temp failure");
		UU_mem_tare(tmp_dir);
		UT_test_assert(UU_dir_create(tmp_dir, S_IRWXU, true) == UU_OK,
					   "mkdir failure");
	}
	
	snprintf(buf, sizeof(buf)/sizeof(buf[0]), "tmp-%ld%d.%s", now, num_tmp,
			 extension ? extension : "tmp");
			 
	ret = tmp_dir;
	while (subdirs && *subdirs) {
		ret = (uu_string_t) UU_path_join_s(ret, *subdirs, NULL);
		UT_test_assert(UU_dir_create(ret, S_IRWXU, true) == UU_OK,
					   "dir create failed");
		ret = track_tmp_file(ret);
		subdirs++;
	}
	ret = track_tmp_file(UU_path_join_s(ret, buf, NULL));
	
	return ret;
}


static uu_string_t track_tmp_file( uu_cstring_t file ) {
	uu_string_t ret;
	
	UT_test_assert(file, "invalid file");
	
	for (int i = 0; i < num_tmp; i++) {
		if (!strcmp(tmp_files[i], file)) {
			return tmp_files[i];
		}
	}
		
	num_tmp++;
	
	tmp_files = UU_mem_realloc(tmp_files, sizeof(uu_string_t) * num_tmp);
	UT_test_assert(tmp_files, "out of memory");
	UU_mem_tare(tmp_files);
	
	tmp_files[num_tmp - 1] = ret = UU_mem_tare(UU_mem_strdup(file));
	UT_test_assert(ret, "out of memory");
	
	return ret;
}


static int tmp_dir_compare(const void *f1, const void *f2) {
	uu_cstring_t *sf1 = (uu_cstring_t *) f1;
	uu_cstring_t *sf2 = (uu_cstring_t *) f2;
	
	UT_test_assert(*sf1 && *sf2, "invalid");
	
	// ...delete from bottom-up.
	return (int) strlen(*sf2) - (int) strlen(*sf1);
}


static void delete_tmp_files( void ) {
	uu_bool_t ok = true;
	
	if (!tmp_dir) {
		return;
	}
	
	for (int i = 0; i < num_tmp; i++) {
		if (UU_file_exists(tmp_files[i]) && unlink(tmp_files[i]) != 0) {
			UT_test_printf("error: failed to delete %s (errno=%d)",
			               tmp_files[i], errno);
			ok = false;
		}
	}
	
	qsort(tmp_files, num_tmp, sizeof(uu_string_t), tmp_dir_compare);
	for (int i = 0; i < num_tmp; i++) {
		if (UU_dir_exists(tmp_files[i]) && rmdir(tmp_files[i]) != 0) {
			UT_test_printf("error: failed to rmdir %s (errno=%d)", tmp_files[i],
			               errno);
			ok = false;
		}
	}
	
	UT_test_assert(ok, "failed to delete temporary files.");
	
	if (UU_dir_exists(tmp_dir) && rmdir(tmp_dir) != 0) {
		UT_test_printf("warning: failed to remove temporary directory %s",
		               tmp_dir);
	}
}
