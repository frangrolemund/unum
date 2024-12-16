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

static uu_csv_t   *csv_new( unsigned cols, size_t buf_len, uu_cstring_t path,
							uu_error_e *err );
static uu_error_e csv_update_path( uu_csv_t *csv, uu_cstring_t path );
static uu_bool_t csv_bnf_FILE( uu_csv_t *csv, uu_error_e *err );
static uu_bool_t csv_bnf_RECORD( uu_csv_t *csv, uu_string_t cur,
                                 uu_string_t *next, uu_error_e *err );
static uu_bool_t csv_bnf_FIELD( uu_string_t cur, uu_string_t *start_field,
                                uu_string_t *end_field, uu_string_t *next,
							    uu_bool_t *is_eol );
static uu_bool_t csv_bnf_COMMA( uu_string_t cur, uu_string_t *next );
static uu_bool_t csv_bnf_CRLF( uu_string_t cur, uu_string_t *next );
static uu_bool_t csv_bnf_CR( uu_string_t cur, uu_string_t *next );
static uu_bool_t csv_bnf_LF( uu_string_t cur, uu_string_t *next );
static uu_bool_t csv_bnf_EOL( uu_string_t cur, uu_string_t *next);


#define MAX_COLS        256
#define ROW_GROUP_SIZE  32


uu_csv_t *UU_csv_open( uu_cstring_t path, uu_error_e *err ) {
	FILE      *fp  = NULL;
	uu_csv_t  *ret = NULL;
	off_t	  len  = UU_file_info(path).st_size;
	
	UU_set_errorp(err, UU_OK);
	
	if (len == 0 || !(fp = fopen(path, "r"))) {
		UU_set_errorp(err, UU_ERR_FILE);
		return NULL;
	}
	
	if (!(ret = csv_new(0, len + 1, path, err)) ||
	    fread(ret->buf, 1, len, fp) != len ||
        !csv_bnf_FILE(ret, err)) {
		fclose(fp);
		UU_csv_delete(ret);
		return NULL;
	}
			
	return ret;
}


uu_csv_t *UU_csv_new( unsigned cols ) {
	return csv_new(cols, 0, NULL, NULL);
}


extern uu_csv_t *UU_csv_memory( uu_cstring_t buf, uu_error_e *err ) {
	uu_csv_t  *ret = NULL;
	
	UU_set_errorp(err, UU_OK);
	
	if (!buf || !*buf ||
		!(ret = csv_new(0, strlen(buf) + 1, NULL, err)) ||
		!strcpy(ret->buf, buf) ||
        !csv_bnf_FILE(ret, err)) {
		UU_csv_delete(ret);
		return NULL;
	}

	return ret;
}


static uu_csv_t *csv_new( unsigned cols, size_t buf_len, uu_cstring_t path,
                          uu_error_e *err ) {
	uu_csv_t     *ret      = NULL;
	const size_t total_len = sizeof(uu_csv_t) + sizeof(char [buf_len]);
	
	ret = UU_malloc(total_len);
	if (!ret) {
		UU_set_errorp(err, UU_ERR_MEM);
		return NULL;
	}
	
	UU_memset(ret, 0, total_len);
	ret->num_cols = cols;
	
	if (csv_update_path(ret, path) != UU_OK) {
		UU_csv_delete(ret);
		UU_set_errorp(err, UU_ERR_MEM);
		return NULL;
	}

	return ret;
}


void UU_csv_delete( uu_csv_t *csv ) {
	if (!csv) {
		return;
	}
	
	if (csv->rows) {
		UU_free(csv->rows);
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
		csv->path = UU_strdup(path);
		if (!csv->path) {
			return UU_ERR_MEM;
		}
	}
	
	return UU_OK;
}


static uu_bool_t csv_bnf_FILE( uu_csv_t *csv, uu_error_e *err ) {
	uu_string_t cur  = csv->buf;
	uu_string_t next = NULL;
	uu_error_e  uerr = UU_OK;
	
	while (csv_bnf_RECORD(csv, cur, &next, &uerr) && next) {
			cur = next;
	}
	
	if ((uerr != UU_OK) || csv->num_rows == 0) {
		UU_set_errorp(err, uerr == UU_OK ? UU_ERR_FMT : uerr);
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
#define is_bnf_EOL(c)        (is_bnf_EOF(c) || is_bnf_CR(c) || is_bnf_LF(c))
#define is_bnf_sep(c)        (is_bnf_COMMA(c) || is_bnf_EOL(c))


static uu_bool_t csv_bnf_RECORD( uu_csv_t *csv, uu_string_t cur,
                                 uu_string_t *next, uu_error_e *err ) {
	uu_string_t  cols[MAX_COLS];
	int          count           = 0;
	uu_string_t  start, end;
	uu_bool_t    is_eol          = false;
	size_t       row_buf_len     = 0;
	uu_string_t  *row_col;

	while (csv_bnf_FIELD(cur, &start, &end, next, &is_eol)) {
		UU_assert(count < MAX_COLS);
		if (count >= MAX_COLS) {
			UU_set_errorp(err, UU_ERR_FMT);
			return false;
		}
		
		if (start && end) {
			*++end     = '\0';
			cols[count] = start;

		} else {
			cols[count] = NULL;
		}
		
		count++;
		cur = *next;
		
		if (is_eol) {
			break;
		}
	}
	
	if ((csv->num_cols == 0 && count == 0) ||
	    (csv->num_cols > 0 && count > 0 && csv->num_cols != count)) {
		UU_set_errorp(err, UU_ERR_FMT);
		return false;
		
	} else if (is_eol && count == 0) {
		return *next ? true : false;	// - allow empty lines
	}
	
	csv->num_cols = count;
	
	if (csv->max_rows <= csv->num_rows) {
		csv->max_rows += ROW_GROUP_SIZE;
		row_buf_len    = sizeof(uu_string_t *) * csv->max_rows * csv->num_cols;
		csv->rows      = (uu_string_t *) UU_realloc(csv->rows, row_buf_len);
		
		if (!csv->rows) {
			UU_set_errorp(err, UU_ERR_MEM);
			return false;
		}
	}

	row_col = csv->rows + (csv->num_rows * csv->num_cols);
	for (int i = 0; i < count; i++) {
		*row_col++ = cols[i];
	}
	
	csv->num_rows++;
	
	return true;
}


// - the litmus test is being able to identify `ccc"cc` below and move to
//   the next in the following invocation:
//   --> aaa,,"ccc""cc",dddddd
static uu_bool_t csv_bnf_FIELD( uu_string_t cur, uu_string_t *start_field,
                                uu_string_t *end_field, uu_string_t *next,
                                uu_bool_t *is_eol ) {
	int         is_quoted = 0;
	uu_string_t pos_esc   = NULL;
	                                
	UU_assert(cur && start_field && end_field && next && is_eol);
	
	*start_field = *end_field = *next = NULL;
	*is_eol      = false;
		
	if ((is_quoted = is_bnf_DQUOTE(*cur)))  {
		cur++;
	}
	
	for (; !(*is_eol = is_bnf_EOF(*cur)) ;) {
		if (is_quoted) {
			if (is_bnf_DQUOTE(*cur)) {
				cur++;

				// - end of field or escaped inner quote?
				if (!is_bnf_DQUOTE(*cur)) {
					if (!is_bnf_sep(*cur)) {
						return false;
					}
					
					is_quoted = 0;
					continue;
				}
				
				if (!pos_esc) {
					pos_esc = cur;
				}
			}

		} else {					
			if (csv_bnf_COMMA(cur, next)) {
				break;
			}

			if ((*is_eol = csv_bnf_EOL(cur, next))) {
				break;
			}
		}
	
		*start_field = *start_field ? *start_field : cur;
		*end_field   = cur++;
	}
	
	if (*is_eol && !start_field) {
		return false;
	}
	
	// - escape compression
	while (pos_esc) {
		cur     = pos_esc;
		pos_esc = NULL;

		for (; cur < *end_field; cur++ ) {
			*cur = *(cur + 1);
			
			if (!pos_esc && is_bnf_DQUOTE(*cur) && is_bnf_DQUOTE(*(cur + 1))) {
				pos_esc = cur + 1;
			}
		}
		
		(*end_field)--;
	}

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

static uu_bool_t csv_bnf_EOL( uu_string_t cur, uu_string_t *next) {
	if (!cur || is_bnf_EOF(*cur)) {
		*next = NULL;
		return true;
	
	}else if (csv_bnf_LF(cur, next)) {
		return true;
		
	} else if (csv_bnf_CRLF(cur, next)) {
		return true;
	}

	return false;
}


unsigned UU_csv_rows( uu_csv_t *csv ) {
	if (csv) {
		return csv->num_rows;
	}
	
	return 0;
}


unsigned UU_csv_cols( uu_csv_t *csv ) {
	if (csv) {
		return csv->num_cols;
	}
	
	return 0;
}


const char *UU_csv_get( uu_csv_t *csv, unsigned row, unsigned col,
                               uu_error_e *err) {
	if (!csv || row >= csv->num_rows || col >= csv->num_cols) {
		UU_set_errorp(err, UU_ERR_ARGS);
		return NULL;
	}
                            
	return *(csv->rows + ((csv->num_cols * row) + col));
}


uu_cstring_t UU_csv_file_path( uu_csv_t *csv ) {
	if (csv && csv->num_rows > 0 && csv->num_cols > 0) {
		return csv->path;
	}

	return NULL;
}
