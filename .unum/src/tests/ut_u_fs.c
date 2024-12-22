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

#include "u_common.h"
#include "u_fs.h"
#include "u_test.h"


static int unittest_fs( int argc, char *argv[] );
static void fs_test_paths( void );


int main( int argc, char *argv[] ) {
	return UT_test(argc, argv, unittest_fs);
}


static int unittest_fs( int argc, char *argv[] ) {
	fs_test_paths();
	return 0;
}


static void fs_test_paths( void ) {
	uu_path_t    path, buf;
	struct stat  s;
	uu_string_t  src, dest, last;

	UT_set_test_name("file paths");
	
	UT_assert(UU_basename(path, U_PATH_MAX, __FILE__) == UU_OK, "failed base");
	UT_assert(!strcmp(path, "ut_u_fs.c"), "failed base");
	UT_printf("file base: %s", path);
	
	UT_assert(UU_dirname(path, U_PATH_MAX, __FILE__) == UU_OK, "failed dir");
	UT_assert(path[strlen(path)-1] == UNUM_PATH_SEP, "failed dir");
	UT_printf("file dir: %s", path);
	
	UT_assert(UU_is_file(__FILE__), "not file");
	s = UU_file_info(__FILE__);
	UT_assert(s.st_size > 0 && s.st_mode & S_IFREG, "failed file info");
	UT_printf("file size: %ld", s.st_size);
	
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
	
	UT_printf("realpath: %s", path);
	dest = (uu_string_t) UU_realpath(path, NULL, NULL);
	UT_assert(!dest, "failed realpath");
	dest = (uu_string_t) UU_realpath(buf, path, NULL);
	UT_assert(!strcmp(dest, __FILE__), "failed realpath");
	UT_assert(dest == buf, "unexpected unused state");
	UT_printf("computed: %s", dest);
	
	while ((dest = (uu_string_t) UU_path_pop(path, U_PATH_MAX, __FILE__))) {
		UT_printf("pop seg: %s", dest);
		last = dest;
	}
	UT_assert(!strcmp(last, __FILE__), "unexpected difference");
}
