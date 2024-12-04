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

static uu_csv_t   *csv_new( unsigned cols, size_t buf_len, const char *path );
static uu_error_e csv_update_path( uu_csv_t *csv, const char *path );


uu_csv_t *UU_csv_open( const char *path, uu_error_e *err ) {
	FILE      *fp  = NULL;
	uu_csv_t  *ret = NULL;
	unsigned  cols = 0;
	off_t	  len  = UU_file_info(path).st_size;
	
	if (len == 0 || !(fp = fopen(path, "r"))) {
		if (err) {
			*err = UU_ERR_FILE;
		}
		return NULL;
	}
	
	if (!(ret = csv_new(0, len + 1, path)) ||
	    fread(ret->buf, 1, len, fp) != len) {
		if (err) {
			*err = ret ? UU_ERR_FILE : UU_ERR_MEM;
		}
		fclose(fp);
		UU_csv_delete(ret);
		return NULL;
	}
			
	return ret;
}


uu_csv_t *UU_csv_new( unsigned cols ) {
	return csv_new(cols, 0, NULL);
}


static uu_csv_t *csv_new( unsigned cols, size_t buf_len, const char *path ) {
	uu_csv_t     *ret      = NULL;
	const size_t total_len = sizeof(uu_csv_t) + sizeof(char [buf_len]);
	
	ret = UU_malloc(total_len);
	if (!ret) {
		return NULL;
	}
	
	UU_memset(ret, 0, total_len);
	ret->num_cols = cols;
	
	if (csv_update_path(ret, path) != UU_OK) {
		UU_csv_delete(ret);
		return NULL;
	}

	return ret;
}


void UU_csv_delete( uu_csv_t *csv ) {
	if (!csv) {
		return;
	}
	
	if (csv->path) {
		UU_free(csv->path);
	}

	UU_free(csv);
}


uu_error_e csv_update_path( uu_csv_t *csv, const char *path ) {
	if (!csv) {
		return UU_ERR_ARGS;
	}
	
	if (csv->path) {
		UU_free(csv->path);
		csv->path = NULL;
	}
	
	if (path) {
		csv->path = strdup(path);
		if (!csv->path) {
			return UU_ERR_MEM;
		}
	}
	
	return UU_OK;
}
