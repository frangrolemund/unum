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

#ifndef UNUM_COMMON_H
#define UNUM_COMMON_H

#include <limits.h>
#include <stdlib.h>

#include "u_config.h"
#include "u_mem.h"

typedef struct {
	int   major;
	int   minor;
	int   patch;
	char  *as_string;
} uu_version_t;

#define UNUM_VERSION ((uu_version_t) { 0, 1, 0, "0.1.0" })


typedef enum {
	UU_OK        = 0,
	UU_ERR_ARGS  = 1,
	UU_ERR_MEM   = 2,
	UU_ERR_FILE  = 3,
	UU_ERR_FMT   = 4
} uu_error_e;


#endif /* UNUM_COMMON_H */
