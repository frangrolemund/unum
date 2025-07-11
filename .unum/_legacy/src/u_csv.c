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

#include <ctype.h>
#include <stdio.h>
#include <string.h>
#include "u_csv.h"

// REF: https://datatracker.ietf.org/doc/html/rfc4180

static uu_csv_t   *csv_new( unsigned cols, size_t buf_len, uu_cstring_t path,
							uu_error_e *err );
static uu_error_e csv_update_path( uu_csv_t *csv, uu_cstring_t path );
static void       csv_delete_field( uu_csv_t *csv, uu_string_t s );
static uu_bool_t  csv_bnf_FILE( uu_csv_t *csv, uu_error_e *err );
static uu_bool_t  csv_bnf_RECORD( uu_csv_t *csv, uu_string_t cur,
                                  uu_string_t *next, uu_error_e *err );
static uu_bool_t  csv_ensure_rows( uu_csv_t *csv );
static uu_bool_t  csv_bnf_FIELD( uu_string_t cur, uu_string_t *start_field,
                                 uu_string_t *end_field, uu_string_t *next,
								 uu_bool_t *is_eol );
static uu_bool_t  csv_bnf_COMMA( uu_string_t cur, uu_string_t *next );
static uu_bool_t  csv_bnf_CRLF( uu_string_t cur, uu_string_t *next );
static uu_bool_t  csv_bnf_CR( uu_string_t cur, uu_string_t *next );
static uu_bool_t  csv_bnf_LF( uu_string_t cur, uu_string_t *next );
static uu_bool_t  csv_bnf_EOL( uu_string_t cur, uu_string_t *next);
static uu_bool_t  csv_req_quote( uu_cstring_t field );
static uu_bool_t  csv_write_field( FILE *fp, uu_cstring_t field,
                                   uu_bool_t comma, uu_bool_t quoted);


#define MAX_COLS        256
#define ROW_GROUP_SIZE  32

#define csv_field(csv, r, c)   (*(csv->rows + ((csv->num_cols * (r)) + (c))))
#define csv_is_file(csv, s)    (s >= (char *)csv &&\
							    s < ((char *)csv + csv->size))


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
	
	ret = UU_mem_alloc(total_len);
	if (!ret) {
		UU_set_errorp(err, UU_ERR_MEM);
		return NULL;
	}
	
	UU_mem_set(ret, 0, total_len);
	ret->num_cols = cols;
	ret->size     = total_len;
	
	if (csv_update_path(ret, path) != UU_OK) {
		UU_csv_delete(ret);
		UU_set_errorp(err, UU_ERR_MEM);
		return NULL;
	}

	return ret;
}


static uu_error_e csv_update_path( uu_csv_t *csv, uu_cstring_t path ) {
	uu_error_e err;
	uu_path_t  dst;
		
	if (!csv) {
		return UU_ERR_ARGS;
	}
	
	if (csv->path) {
		if (path && !strcmp(csv->path, path)) {
			return UU_OK;
		}

		UU_mem_free(csv->path);
		csv->path = NULL;
	}
	
	if (path) {
		if (!(path = UU_path_normalize(dst, path, &err))) {
			return err;
		}
	
		csv->path = UU_mem_strdup(path);
		if (!csv->path) {
			return UU_ERR_MEM;
		}
	}
	
	return UU_OK;
}


void UU_csv_delete( uu_csv_t *csv ) {
	if (!csv) {
		return;
	}
	
	for (int i = 0; i < csv->num_rows; i++) {
		for (int j = 0; j < csv->num_cols; j++) {
			csv_delete_field(csv, csv_field(csv, i, j));
		}
	}
	
	if (csv->rows) {
		UU_mem_free(csv->rows);
	}
	
	if (csv->path) {
		UU_mem_free(csv->path);
	}

	UU_mem_free(csv);
}


static void csv_delete_field( uu_csv_t *csv, uu_string_t s ) {
	if (s && !csv_is_file(csv, s)) {
		UU_mem_free(s);
	}
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

	while (csv_bnf_FIELD(cur, &start, &end, next, &is_eol)) {
		assert(count < MAX_COLS);
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
	
	if (!csv_ensure_rows(csv)) {
		UU_set_errorp(err, UU_ERR_MEM);
		return false;
	}

	for (int i = 0; i < count; i++) {
		csv_field(csv, csv->num_rows, i) = cols[i];
	}
	
	csv->num_rows++;
	
	return true;
}

static uu_bool_t csv_ensure_rows( uu_csv_t *csv ) {
	size_t row_buf_len = 0;

	if (csv->max_rows <= csv->num_rows) {
		csv->max_rows += ROW_GROUP_SIZE;
		row_buf_len    = sizeof(uu_string_t *) * csv->max_rows * csv->num_cols;
		csv->rows      = (uu_string_t *) UU_mem_realloc(csv->rows, row_buf_len);
		
		if (!csv->rows) {
			return false;
		}
	}
	
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
	                                
	assert(cur && start_field && end_field && next && is_eol);
	
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
	assert(cur && next);
	
	if (is_bnf_COMMA(*cur)) {
		*next = ++cur;
		return true;
	}
	
	return false;
}


static uu_bool_t csv_bnf_CRLF( uu_string_t cur, uu_string_t *next ) {
	uu_string_t tnx = NULL;
	
	assert(cur && next);
	
	if (csv_bnf_CR(cur, &tnx) && csv_bnf_LF(tnx, next)) {
		return true;
	}
	
	return false;
}


static uu_bool_t csv_bnf_CR( uu_string_t cur, uu_string_t *next ) {
	assert(cur && next);
	
	if (is_bnf_CR(*cur)) {
		*next = ++cur;
		return true;
	}
	
	return false;
}


static uu_bool_t csv_bnf_LF( uu_string_t cur, uu_string_t *next ) {
	assert(cur && next);
	
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


unsigned UU_csv_row_count( uu_csv_t *csv ) {
	if (csv) {
		return csv->num_rows;
	}
	
	return 0;
}


unsigned UU_csv_col_count( uu_csv_t *csv ) {
	if (csv) {
		return csv->num_cols;
	}
	
	return 0;
}


uu_cstring_t UU_csv_get( uu_csv_t *csv, unsigned row, unsigned col,
					     uu_error_e *err) {
	if (!csv || row >= csv->num_rows || col >= csv->num_cols) {
		UU_set_errorp(err, UU_ERR_ARGS);
		return NULL;
	}
                            
	return csv_field(csv, row, col);
}


uu_error_e UU_csv_set( uu_csv_t *csv, unsigned row, unsigned col,
					   uu_cstring_t value ) {
	uu_string_t field = NULL;
					   
	if (!csv || row >= csv->num_rows || col >= csv->num_cols) {
		return UU_ERR_ARGS;
	}
	
	if (value) {
		field = UU_mem_strdup(value);
		if (!field) {
			return UU_ERR_MEM;
		}
	}
	
	csv_delete_field(csv, csv_field(csv, row, col));
	csv_field(csv, row, col) = field;

	return UU_OK;
}


uu_cstring_t UU_csv_file_path( uu_csv_t *csv ) {
	if (csv && csv->num_rows > 0 && csv->num_cols > 0) {
		return csv->path;
	}

	return NULL;
}


uu_error_e UU_csv_write( uu_csv_t *csv, uu_cstring_t path ) {
	FILE         *fp   = NULL;
	uu_cstring_t field;
	uu_bool_t    req_quote;
	uu_error_e	 ret   = UU_OK;
	
	if (!csv || !csv->num_rows || !csv->num_cols) {
		return UU_ERR_ARGS;
	}
	
	path = path ? path : csv->path;
	if (!path) {
		return UU_ERR_ARGS;
	}
	
	fp = fopen(path, "w");
	if (!fp) {
		return UU_ERR_FILE;
	}
	
	for (int i = 0; i < csv->num_rows; i++) {
		for (int j = 0; j < csv->num_cols; j++) {
			field     = csv_field(csv, i, j);
			req_quote = csv_req_quote(field);
			
			if (!csv_write_field(fp, field, j > 0, req_quote)) {
				ret = UU_ERR_FILE;
				goto write_done;
			}
		}
		
		fprintf(fp, "\n");
	}
	
write_done:
	fclose(fp);
	
	return csv_update_path(csv, path);
}


static uu_bool_t csv_req_quote( uu_cstring_t field ) {
	char c;
	
	while (field && *field && (c = *field++)) {
		if (c == ',' || isspace(c)) {
			return true;
		}
	}

	return false;
}


static uu_bool_t csv_write_field( FILE *fp, uu_cstring_t field,
                                  uu_bool_t comma, uu_bool_t quoted) {
	if (comma && !fputc(',', fp)) {
		return false;
	}
	
	if (quoted && !fputc('\"', fp)) {
		return false;
	}
	
	for (; field && *field ; field++) {
		if (!fputc(*field, fp)) {
			return false;
		}
		
		if (*field == '\"' && !fputc(*field, fp)) {
			return false;
		}
	}
		
	if (quoted && !fputc('\"', fp)) {
		return false;
	}
	
	return true;
}


unsigned UU_csv_add_row( uu_csv_t *csv ) {
	if (!csv) {
		return 0;
	}
	
	return UU_csv_insert_row(csv, csv->num_rows) == UU_OK ? csv->num_rows : 0;
}


uu_error_e UU_csv_insert_row( uu_csv_t *csv, unsigned offset ) {
	if (!csv || offset > csv->num_rows) {
		return UU_ERR_ARGS;
	}
	
	csv->num_rows++;
	
	if (!csv_ensure_rows(csv)) {
		csv->num_rows--;
		return UU_ERR_MEM;
	}
	
	if (csv->num_rows > 1) {
		for (int i = csv->num_rows - 1; i > offset; i--) {
			for (int j = 0; j < csv->num_cols; j++) {
				csv_field(csv, i, j) = csv_field(csv, i - 1, j);
			}
		}
	}
	
	for (int j = 0; j < csv->num_cols; j++) {
		csv_field(csv, offset, j) = NULL;
	}

	return UU_OK;
}


uu_error_e UU_csv_delete_row( uu_csv_t *csv, unsigned row ) {
	if (!csv || row >= csv->num_rows) {
		return UU_ERR_ARGS;
	}

	for (int j = 0; j < csv->num_cols; j++) {
		csv_delete_field(csv, csv_field(csv, row, j));
	}
	
	for (; row < csv->num_rows - 1 ; row++) {
		for (int j = 0; j < csv->num_cols; j++) {
			csv_field(csv, row, j) = csv_field(csv, row + 1, j);
		}
	}
	
	for (int j = 0; j < csv->num_cols; j++) {
		csv_field(csv, row, j) = NULL;
	}
	
	csv->num_rows--;
	
	return UU_OK;
}
