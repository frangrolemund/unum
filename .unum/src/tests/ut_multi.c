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

/*
 *  Runs all unit tests.
 *  - as a matter of principle and some practicality, this doesn't
 *    use the test framework to remain externally objective and able
 *    to make finer-grained decisions about results.
 */

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <unistd.h>

#include "u_common.h"
#include "u_test.h"
#include "u_fs.h"
#include "u_time.h"


typedef struct {
	uu_cstring_t name;
	int          status;

} multi_result_t;


static void multi_print( char *fmt, ... );
static void multi_vprint( char *fmt, va_list ap );
static void multi_parse_cmdline( int argc, char *argv[] );
static void multi_abort( char *msg, ... );
static void multi_test( void );
static void *multi_alloc( size_t len );
static void multi_test_print( multi_result_t *r, uu_string_t fmt, ... );
static void multi_test_run( multi_result_t *r );
static void multi_report( void );


uu_path_t          test_dir;
uu_cstring_t const sub_tests[] = {
	"ut_u_mem",
	"ut_u_fs",
	"ut_u_proc",
	"ut_u_csv",
	"ut_d_manifest"
};
const int          num_tests   = sizeof(sub_tests) / sizeof(sub_tests[0]);
uu_time_mark_t     start;
size_t 			   max_tlen    = 0;
multi_result_t     **results   = NULL;


int main( int argc, char *argv[] ) {
	start = UU_time_mark_ns();
	
	multi_parse_cmdline(argc, argv);
	multi_test();
	multi_report();
}


static void multi_print( char *fmt, ... ) {
	va_list     ap;
	
	va_start(ap, fmt);
	multi_vprint(fmt, ap);
	va_end(ap);
}


static void multi_vprint( char *fmt, va_list ap ) {
	vfprintf(stdout, fmt, ap);
	fprintf(stdout, "\n");
}


static void multi_parse_cmdline( int argc, char *argv[] ) {
	if (!argc ||
	    UU_path_dirname(test_dir, sizeof(test_dir), argv[0]) != UU_OK) {
		multi_abort("failed to identify the test directory.");
	}
}


static void multi_abort( char *msg, ... ) {
	va_list ap;
	
	va_start(ap, msg);
	fprintf(stderr, "ut_multi: ");
	vfprintf(stderr, msg, ap);
	fprintf(stderr, "\n");
	va_end(ap);

	exit(1);
}


static void multi_test( void ) {
	multi_result_t *r;
	size_t         len;
	
	results = multi_alloc(sizeof(multi_result_t *) * num_tests);
	for (int i = 0; i < num_tests; i++) {
		len = strlen(sub_tests[i]);
		if (len > max_tlen) {
			max_tlen = len;
		}
	}

	for (int i = 0; i < num_tests; i++) {
		results[i] = r = multi_alloc(sizeof(multi_result_t));
		UU_mem_reset(r, sizeof(multi_result_t));
		r->name = sub_tests[i];
		multi_test_run(r);
	}
}


static void *multi_alloc( size_t len ) {
	void *ret;
	
	ret = UU_mem_alloc(len);
	if (!ret) {
		multi_abort("out of memory");
	}
	return ret;
}


static void multi_test_print( multi_result_t *r, uu_string_t fmt, ... ) {
	va_list      ap;
	char         pfx[64]  = { '\0' };
	char         buf[256];
	
	snprintf(pfx, sizeof(pfx), "[%s]", r->name);
	
	va_start(ap, fmt);
	vsnprintf(buf, sizeof(buf), fmt, ap);
	va_end(ap);
	multi_print("%*s  %s", -(max_tlen+2), pfx, buf);
}


static void multi_test_run( multi_result_t *r ) {
	multi_test_print(r, "complete");
}


static void multi_report( void ) {
	multi_print("");
	multi_print("%*s  %s", -(max_tlen+2), "Elapsed:",
	            UU_time_mark_delta_s(start).desc);
}
