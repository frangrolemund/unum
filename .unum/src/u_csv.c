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
#include "u_csv.h"

uu_csv_t *UU_csv_open( const char *path, uu_error_e *err ) {
	FILE      *fp  = NULL;
	uu_csv_t  *ret = NULL;
	unsigned  cols = 0;
	
	fp = fopen(path, "r");
	if (!fp) {
		if (err) {
			*err = UU_ERR_FILE;
		}
		return NULL;
	}
	
	ret = UU_csv_new(1);
		
	return ret;
}


uu_csv_t *UU_csv_new( unsigned cols ) {
	uu_csv_t *ret = NULL;
	
	ret = UU_malloc(sizeof(uu_csv_t));
	if (!ret) {
		return NULL;
	}
	
	UU_memset(ret, 0, sizeof(uu_csv_t));
	ret->num_cols = cols;

	return ret;
}
