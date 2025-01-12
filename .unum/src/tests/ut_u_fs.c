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

#include <unistd.h>

#include "u_common.h"
#include "u_fs.h"
#include "u_test.h"


static int unittest_fs( int argc, char *argv[] );
static void fs_test_paths( void );
static void fs_test_dirs( void );


int main( int argc, char *argv[] ) {
	return UT_test_run(argc, argv, unittest_fs);
}


static int unittest_fs( int argc, char *argv[] ) {
	fs_test_paths();
	fs_test_dirs();
	return 0;
}


static void fs_test_paths( void ) {
	uu_path_t    path, buf;
	struct stat  s;
	uu_string_t  src, dest, last;

	UT_test_setname("file paths");
	
	UT_test_assert(UU_path_basename(path, U_PATH_MAX, __FILE__) == UU_OK,
	               "failed base");
	UT_test_assert_eq(path, "ut_u_fs.c", "failed base");
	UT_test_printf("file base: %s", path);
	
	UT_test_assert(UU_path_dirname(path, U_PATH_MAX, __FILE__) == UU_OK,
	               "failed dir");
	UT_test_assert(path[strlen(path)-1] == UNUM_PATH_SEP, "failed dir");
	UT_test_printf("file dir: %s", path);
	UT_test_assert(UU_path_normalize(path, path, NULL), "failed normalize");
	UT_test_assert(path[strlen(path)-1] != UNUM_PATH_SEP, "failed dir");
	
	UT_test_assert(UU_file_exists(__FILE__), "not file");
	s = UU_file_info(__FILE__);
	UT_test_assert(s.st_size > 0 && s.st_mode & S_IFREG, "failed file info");
	UT_test_printf("file size: %ld", s.st_size);
	
	src  = __FILE__;
	dest = path;
	do {
		if (*src == UNUM_PATH_SEP) {
			*dest++ = *src;
			if ((unsigned long) src & 0x01) {
				*dest++ = '.';
			}
		}
		*dest++ = *src++;
	} while (*src);
	*dest = '\0';
	
	UT_test_printf("realpath: %s", path);
	dest = (uu_string_t) UU_path_normalize(path, NULL, NULL);
	UT_test_assert(!dest, "failed realpath");
	dest = (uu_string_t) UU_path_normalize(buf, path, NULL);
	UT_test_assert_eq(dest, __FILE__, "failed realpath");
	UT_test_assert(dest == buf, "unexpected unused state");
	UT_test_printf("computed: %s", dest);
	
	while ((dest = (uu_string_t) UU_path_prefix(path, U_PATH_MAX, __FILE__))) {
		UT_test_printf("pop seg: %s", dest);
		last = dest;
	}
	UT_test_assert_eq(last, __FILE__, "unexpected difference");
	
	UT_test_assert(!UU_path_to_independent(buf, U_PATH_MAX, "/foo/bar"),
	               "failed to detect error");
	UT_test_assert_eq(UU_path_to_independent(buf, U_PATH_MAX, ".\\foo\\bar"),
	               "./foo/bar", "failed to convert");
	
	
	#if !UNUM_OS_UNIX
	// - the path checks below will need to be reworked.
	#error "Not yet supported."
	#endif
	
	UT_test_assert_eq(UU_path_join(buf, U_PATH_MAX,
	             "\0", "./abc", "\0", "def", NULL),
	             "./abc/def", "invalid join");
	             
	UT_test_assert(UU_path_join(buf, 5, "bike", "X", NULL) == NULL,
	               "invalid join");
	UT_test_assert(UU_path_join(buf, 0, "Cd", "De", "Za", NULL) == NULL,
	               "invalid join");
	               
	UT_test_assert_eq(UU_path_to_platform(buf, U_PATH_MAX, "./foo/bar"),
	               "./foo/bar", "failed to convert");	               
}

 
void fs_test_dirs( void ) {
	uu_path_t tmpdir;
	
	UT_test_setname("directories");
	
	UT_test_assert(UU_path_join(tmpdir, U_PATH_MAX, UNUM_DIR_TEST, "a", "b",
	          "c", "d", NULL), "path_join_failed");
	UT_test_assert(UU_file_none(tmpdir), "dir exists");
	UT_test_assert(UU_dir_create(tmpdir, S_IRWXU, false) != UU_OK, "created dir?");
	UT_test_assert(UU_file_none(tmpdir), "dir exists");
		
	UT_test_assert(UU_dir_create(tmpdir, S_IRWXU, true) == UU_OK,
	               "failed to create");
	UT_test_assert(UU_dir_exists(tmpdir), "failed to create");
	UT_test_printf("created directory %s", tmpdir);
	UT_test_assert(UU_dir_create(tmpdir, S_IRWXU, true) == UU_OK, "not idempotent");
	UT_test_assert(UU_dir_exists(tmpdir), "failed to create");
	
	UT_test_assert(rmdir(tmpdir) == 0, "cannot remove dir");
	UT_test_assert(rmdir(UU_path_join_s(UNUM_DIR_TEST, "a", "b", "c",
	                                    NULL)) == 0, "cannot remove dir");
	UT_test_assert(rmdir(UU_path_join_s(UNUM_DIR_TEST, "a", "b", NULL)) == 0,
	               "cannot remove dir");
	UT_test_assert(rmdir(UU_path_join_s(UNUM_DIR_TEST, "a", NULL)) == 0,
	               "cannot remove dir");
}
