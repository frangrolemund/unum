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

#ifndef UNUM_TIME_H
#define UNUM_TIME_H


#include "u_common.h"


typedef uint64_t     uu_time_mark_t;
#define UU_NS_IN_SEC 1000000000

typedef struct {
	uu_string_t    desc;
	uu_time_mark_t start;
	uu_time_mark_t end;
} uu_time_delta_t;


/*
 * UU_time_mark_ns()
 * - returns the value of the system clock as a number of nanoseconds from some
 *   arbitrary point, intended to be used for computing high-precision offsets.
 */
extern uu_time_mark_t UU_time_mark_ns( void );

/*
 * UU_time_mark_delta()
 * - populates and returns a time difference from the `start` until the
 *   present moment expressed in a fraction of seconds or a smaller unit when
 *   necessary.  The provided `buf` is returned in the `desc` attribute of
 *   the returned structure or assigned NULL if the `buf` is not large enough.
 */
extern uu_time_delta_t UU_time_mark_delta( uu_time_mark_t start, uu_string_t buf,
									       size_t len );


/*
 * UU_time_mark_delta_s()
 * - populates and returns a time difference (with static description buffer)
 *   from the `start` until the present moment expressed in a fraction of
 *   seconts or smaller unit when necessary.
 */
extern uu_time_delta_t UU_time_mark_delta_s( uu_time_mark_t start );


/*
 * UU_time_mark_delta_ns()
 * - returns the number of nanoseconds difference in a time delta.
 */
#define UU_time_mark_delta_ns(td) ((td).end - (td).start)


/*
 * UU_time_millisleep()
 * - sleep for a number of milliseconds expressed by `ms`.  Returns `false`
 *   if interrupted by a signal or other system event.
 */
extern uu_bool_t UU_time_millisleep( unsigned ms );


#endif /* UNUM_TIME_H */
