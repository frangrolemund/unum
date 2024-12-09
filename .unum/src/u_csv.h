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

#ifndef UNUM_CSV_H
#define UNUM_CSV_H

#include "u_common.h"
#include "u_fs.h"

typedef char ** uu_csv_row_t;
typedef struct {
	unsigned     num_cols;   // const
	unsigned     num_rows;
	
	uu_csv_row_t *rows;
	unsigned     max_rows;
	
	uu_string_t  path;
	char         buf[];
} uu_csv_t;

/*
 * UU_csv_new()
 * - create a new in-memory CSV instance to be later freed with UU_csv_delete().
 */
extern uu_csv_t   *UU_csv_new( unsigned cols );


/*
 * UU_csv_open()
 * - open an existing CSV file from the provided path to be later freed with
 *   UU_csv_delete().
 */
extern uu_csv_t   *UU_csv_open( uu_cstring_t path, uu_error_e *err );


/*
 * UU_csv_memory()
 * - open an existing CSV file from the provided buffer to be later freed with
 *   UU_csv_delete().
 */
extern uu_csv_t   *UU_csv_memory( uu_cstring_t buf, uu_error_e *err );


/*
 * UU_csv_write()
 * - write the CSV file to the path or NULL to use the same path as input.
 */
extern int        UU_csv_write( uu_cstring_t path );


/*
 * UU_csv_delete()
 * - free resources for CSV instance.
 */
extern void       UU_csv_delete( uu_csv_t *csv );

/*
 * UU_csv_add_row()
 * - add a new row to the table.
 */
extern int        UU_csv_add_row( uu_csv_t *csv );


/*
 * UU_csv_get()
 * - get the value from a specific cell in the CSV file.
 */
extern const char *UU_csv_get( uu_csv_t *csv, unsigned row, unsigned col,
                               int *errc );


/*
 * UU_csv_set()
 * - assign the value of a specific cell in the CSV file.
 */
extern int        UU_csv_set( uu_csv_t *csv, unsigned row, unsigned col,
                              uu_string_t *value );


/*
 * UU_csv_cols()
 * - get the number of columns in the CSV file.
 */
extern unsigned   UU_csv_cols( uu_csv_t *csv );


/*
 * UU_csv_rows()
 * - get the number of rows in the CSV file.
 */
extern unsigned   UU_csv_rows( uu_csv_t *csv );


#endif /* UNUM_CSV_H */
