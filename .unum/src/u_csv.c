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
#include <string.h>
#include "u_csv.h"

// REF: https://datatracker.ietf.org/doc/html/rfc4180

static uu_csv_t   *csv_new( unsigned cols, size_t buf_len, uu_cstring_t path );
static uu_error_e csv_update_path( uu_csv_t *csv, uu_cstring_t path );
static uu_bool_t csv_bnf_FILE( uu_csv_t *csv );
static uu_bool_t csv_bnf_RECORD( uu_csv_t *csv, uu_string_t cur,
                                 uu_string_t *next );
static uu_bool_t csv_bnf_FIELD( uu_string_t cur, uu_string_t *start_field,
                                uu_string_t *end_field, uu_string_t *next );
static uu_bool_t csv_bnf_COMMA( uu_string_t cur, uu_string_t *next );
static uu_bool_t csv_bnf_CRLF( uu_string_t cur, uu_string_t *next );
static uu_bool_t csv_bnf_CR( uu_string_t cur, uu_string_t *next );
static uu_bool_t csv_bnf_LF( uu_string_t cur, uu_string_t *next );

uu_csv_t *UU_csv_open( uu_cstring_t path, uu_error_e *err ) {
	FILE      *fp  = NULL;
	uu_csv_t  *ret = NULL;
	off_t	  len  = UU_file_info(path).st_size;
	
	UU_set_errorp(err, UU_OK);
	
	if (len == 0 || !(fp = fopen(path, "r"))) {
		UU_set_errorp(err, UU_ERR_FILE);
		return NULL;
	}
	
	if (!(ret = csv_new(0, len + 1, path)) ||
	    fread(ret->buf, 1, len, fp) != len ||
		!csv_bnf_FILE(ret)) {
		UU_set_errorp(err, ret ? UU_ERR_FILE : UU_ERR_MEM);
		fclose(fp);
		UU_csv_delete(ret);
		return NULL;
	}
			
	return ret;
}


uu_csv_t *UU_csv_new( unsigned cols ) {
	return csv_new(cols, 0, NULL);
}


extern uu_csv_t *UU_csv_memory( uu_cstring_t buf, uu_error_e *err ) {
	uu_csv_t  *ret = NULL;
	
	UU_set_errorp(err, UU_OK);
	
	if (!buf || !*buf ||
		!(ret = csv_new(0, strlen(buf) + 1, NULL)) ||
		!strcpy(ret->buf, buf) ||
		!csv_bnf_FILE(ret)) {
		UU_set_errorp(err, ret ? UU_ERR_ARGS : UU_ERR_MEM);
		UU_csv_delete(ret);
		return NULL;
	}

	return ret;
}


static uu_csv_t *csv_new( unsigned cols, size_t buf_len, uu_cstring_t path ) {
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


static uu_error_e csv_update_path( uu_csv_t *csv, uu_cstring_t path ) {
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


static uu_bool_t csv_bnf_FILE( uu_csv_t *csv ) {
	uu_string_t cur  = csv->buf;
	uu_string_t next = NULL;
	
	while (csv_bnf_RECORD(csv, cur, &next) && *next)
		{}
	
	if (csv->num_cols == 0 || csv->num_rows == 0 || !next) {
		return false;
	}
	
	return true;
}


//  BNF: returns `true` and the location of the character immediately after
//       it if the pattern matches.

#define is_bnf_COMMA(c)      (c == 0x2C)
#define is_bnf_CR(c)         (c == 0x0D)
#define is_bnf_LF(c)         (c == 0x0A)
#define is_bnf_DQUOTE(c)     (c == 0x22)
#define is_bnf_EOF(c)        (c == 0x00)
#define is_bnf_sep(c)        (is_bnf_EOF(c) || is_bnf_COMMA(c) || \
                             is_bnf_CR(c) || is_bnf_LF(c))


static uu_bool_t csv_bnf_RECORD( uu_csv_t *csv, uu_string_t cur,
                                 uu_string_t *next ) {

	return false;
}


// - the litmus test is being able to identify `ccc"cc` below and move to
//   the next in the following invocation:
//   --> aaa,,"ccc""cc",dddddd
static uu_bool_t csv_bnf_FIELD( uu_string_t cur, uu_string_t *start_field,
                                uu_string_t *end_field, uu_string_t *next ) {
	int is_quoted = 0;
	                                
	UU_assert(cur && start_field && end_field && next);
	
	*start_field = *end_field = *next = NULL;
		
	if ((is_quoted = is_bnf_DQUOTE(*cur)))  {
		cur++;
	}
	
	for (; !is_bnf_EOF(*cur) ;) {
		if (is_quoted && is_bnf_DQUOTE(*cur)) {
			cur++;
				
			// ...only two options after finding a second quote
			if (!is_bnf_DQUOTE(*cur) && !is_bnf_sep(*cur)) {
				return false;
			}
				
			is_quoted = 0;
		}
		
		if (!is_quoted && is_bnf_sep(*cur)) {
			break;
		}
	
		*start_field = *start_field ? *start_field : cur;
		*end_field   = cur++;
	}

	*next = is_bnf_EOF(*cur) ? NULL : ++cur;
                                
	return !is_quoted && (*start_field || *next);
}


static uu_bool_t csv_bnf_COMMA( uu_string_t cur, uu_string_t *next ) {
	UU_assert(cur && next);
	
	if (is_bnf_COMMA(*cur)) {
		*next = ++cur;
		return true;
	}
	
	return false;
}


static uu_bool_t csv_bnf_CRLF( uu_string_t cur, uu_string_t *next ) {
	uu_string_t tnx = NULL;
	
	UU_assert(cur && next);
	
	if (csv_bnf_CR(cur, &tnx) && csv_bnf_LF(tnx, next)) {
		return true;
	}
	
	return false;
}


static uu_bool_t csv_bnf_CR( uu_string_t cur, uu_string_t *next ) {
	UU_assert(cur && next);
	
	if (is_bnf_CR(*cur)) {
		*next = ++cur;
		return true;
	}
	
	return false;
}


static uu_bool_t csv_bnf_LF( uu_string_t cur, uu_string_t *next ) {
	UU_assert(cur && next);
	
	if (is_bnf_LF(*cur)) {
		*next = ++cur;
		return true;
	}
	
	return false;
}
