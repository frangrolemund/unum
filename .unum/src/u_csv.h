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

typedef struct {

} uu_csv_t;

/*
 * UU_csv_new()
 * - create a new in-memory CSV instance to be later freed with UU_csv_delete().
 */
extern uu_csv_t  *UU_csv_new( void );


/*
 * UU_csv_open()
 * - open an existing CSV file from the provided path to be later freed with
 *   UU_csv_delete().
 */
extern uu_csv_t  *UU_csv_open( const char *path );


/*
 *  UU_csv_delete()
 * - free resources for CSV instance.
 */
extern void      UU_csv_delete( uu_csv_t *csv );


#endif /* UNUM_CSV_H */
