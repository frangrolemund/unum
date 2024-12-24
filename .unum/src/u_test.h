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

#ifndef UNUM_TEST_H
#define UNUM_TEST_H

#include <stdio.h>
#include "u_common.h"

#ifndef UNUM_UNIT_TEST
#error "Unit testing only."
#endif

#define UNUM_DIR_TEST  UNUM_DIR_DEPLOY UNUM_PATH_SEP_S "test"


/*
 * UT_assert()
 * - verify a test assertion and abort if it failed.
 */
#define UT_assert( t, m ) if (!(t)) {\
	                          _UT_test_failed(#t, __FILE__, __LINE__, (m));\
						  }
extern void _UT_test_failed( uu_cstring_t expr, uu_cstring_t file, int line,
							 uu_cstring_t msg);
							 
#define UT_assert_eq( s1, s2, d ) UT_assert(!strcmp((s1), (s2)), (d))


/*
 * UT_printf()
 * - display a unit test message using printf syntax.
 */
extern void UT_printf( uu_cstring_t fmt, ... );


/*
 * UT_rel_file()
 * - converts a relative filename co-located with the test into an absolute
 *   file path.
 */
extern char *UT_rel_file( uu_cstring_t file );


/*
 * UT_set_name()
 * - assign a name to the current test.
 */
extern void UT_set_name( uu_cstring_t test_name );


/*
 * UT_test()
 * - execute test in sandbox and cleanup after
 */
#define UT_test( c, v, f )     _UT_test(__FILE__, (c), (v), (f))
typedef int (*UT_test_entry_t)( int argc, uu_string_t argv[] );
extern int  _UT_test( uu_cstring_t file, int argc, uu_string_t argv[],
					  UT_test_entry_t entry_fn );
					  
					  
/*
 * UT_tempnam()
 * - returns a temporary filename in a static variable.
 */
extern uu_cstring_t UT_tmpnam( void );


#endif /* UNUM_TEST_H */
