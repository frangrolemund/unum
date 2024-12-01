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
#include "common.h"

#ifndef UNUM_UNIT_TEST
#error "Unit testing only."
#endif

/*
 * UT_assert()
 * - verify a test assertion and abort if it failed.
 */
#define UT_assert(t, m)    if (!(t)) {\
	                           _UT_assert_failed(#t, __FILE__, __LINE__, (m));\
	                       }
extern void _UT_assert_failed(const char *expr, const char *file, int line,
                              const char *msg);


/*
 * UT_run()
 * - execute test in sandbox and cleanup after
 */
#define UT_run(c, v, f)    _UT_run(__FILE__, (c), (v), (f))
typedef int (*UT_test_entry_t)(int argc, char *argv[]);
extern int  _UT_run(const char *file, int argc, char *argv[],
					UT_test_entry_t entry_fn);


#endif /* UNUM_TEST_H */
