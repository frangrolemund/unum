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

#include <stdio.h>
#include <time.h>

#include "u_common.h"
#include "u_time.h"

#define TBUF_LEN  32

uu_time_mark_t UU_time_mark_ns( void ) {
#if UNUM_OS_UNIX
	struct timespec ts = {0, 0};
	clock_gettime(CLOCK_MONOTONIC, &ts);
	return (((uu_time_mark_t)ts.tv_sec) * UU_NS_IN_SEC) + ts.tv_nsec;
	
#else
#error "Not implemented."
#endif
}


extern uu_time_delta_t UU_time_mark_delta( uu_time_mark_t start,
                                           uu_string_t buf, size_t len ) {
	uu_time_delta_t ret            = { NULL, 0, 0 };
	uu_time_mark_t  delta;
	double          delta_f;
	char            tmp[TBUF_LEN];
	unsigned        val;
	
	ret.end   = UU_time_mark_ns();
	ret.start = start;
	delta     = UU_time_mark_delta_ns(ret);
	
	delta_f   = (double) delta / (double) UU_NS_IN_SEC;
	snprintf(tmp, sizeof(tmp), "%.4fs", delta_f);
	
	if (!strcmp(tmp, "0.0000s")) {
		if ((val = (unsigned) delta / (unsigned) 1000000)) {
			snprintf(tmp, sizeof(tmp), "%dms", val);
		} else if (delta) {
			snprintf(tmp, sizeof(tmp), "%dμs",
			         (unsigned) delta / (unsigned) 1000);
		} else {
			strcpy(tmp, "0s");
		}
	}
	
	if (strlen(tmp) < len) {
		strcpy(buf, tmp);
		ret.desc = buf;
	}
	
	return ret;
}


extern uu_time_delta_t UU_time_mark_delta_s( uu_time_mark_t start ) {
	static char buf[TBUF_LEN];
	return UU_time_mark_delta(start, buf, sizeof(buf));
}


extern uu_bool_t UU_time_millisleep( unsigned ms ) {
#if UNUM_OS_UNIX
	struct timespec ts;
	
	UU_mem_reset(&ts, sizeof(ts));
	ts.tv_nsec = (long) ms * 1000000;
	return nanosleep(&ts, NULL) != -1;
	
#else
#error "Not implemented."
#endif
}
