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

#ifndef UNUM_MANIFEST_H
#define UNUM_MANIFEST_H

#include "u_common.h"
#include "u_csv.h"


/*
 *  Systemic manifest:
 *  - accounting and dependency relationships for project source files in
 *    platform-independent format
 *  - supports manual editing where necessary (and done carefully)
 *  - simple access patterns to ensure correctness
 */


#define U_MANIFEST_MAX_NAME 256


typedef enum {
	UD_MANP_CORE = 0,   // ... bootstrapping
	UD_MANP_KERN,       // ... kernel runtime (requires core)
	UD_MANP_CUSTOM,     // ... non-kernel systems (requires kern)
	UD_MANP_TEST        // ... unit testing (requires custom, kern or core)
} ud_manifest_phase_e;


typedef struct {
	uu_path_t root;
	uu_csv_t  *csv;
} ud_manifest_t;


typedef struct {
	uu_cstring_t        path;
	ud_manifest_phase_e phase;
	ud_manifest_phase_e req;
	uu_cstring_t        name;       // - ignored for non-test files
	
	// - reserved
	uu_path_t           res1;
	char                res2[U_MANIFEST_MAX_NAME];
} ud_manifest_file_t;


/*
 * UD_manifest_new()
 * - create an empty manifest while assuming files are relative to the
 *   provided root directory.
 */
extern ud_manifest_t *UD_manifest_new( uu_cstring_t root, uu_error_e *err );


/*
 * UD_manifest_open()
 * - open an existing manifest from a file within a branch of the provided root
 *   directory, validating its contents.
 */
extern ud_manifest_t *UD_manifest_open( uu_cstring_t root, uu_cstring_t path,
                                        uu_error_e *err );


/*
 * UD_manifest_add_file()
 * - add/update a file to the manifest.
 */
extern uu_error_e    UD_manifest_add_file( ud_manifest_t *man,
                                           ud_manifest_file_t file );


/*
 * UD_manifest_file_count()
 * - return the number of files in the manifest.
 */
extern unsigned      UD_manifest_file_count( ud_manifest_t *man );


/*
 * UD_manifest_get()
 * - get a manifest file definition at the given `index` offset.
 */
extern uu_bool_t     UD_manifest_get( ud_manifest_t *man, unsigned index,
                                      ud_manifest_file_t *file );


/*
 * UD_manifest_delete_file()
 * - delete a file from the manifest identified by path.
 */
extern uu_error_e    UD_manifest_delete_file( ud_manifest_t *man,
											  uu_cstring_t path );


/*
 * UD_manifest_delete_file()
 * - delete a file from the manifest identified by index.
 */
extern uu_error_e    UD_manifest_delete_file_n( ud_manifest_t *man,
											    unsigned index );

/*
 * UD_manifest_root()
 * - return the root for the manifest.
 */
extern uu_cstring_t  UD_manifest_root( ud_manifest_t *man );


/*
 * UU_manifest_write()
 * - write the manifest to the path or NULL to use the same path as input.
 */
extern uu_error_e    UD_manifest_write( ud_manifest_t *man, uu_cstring_t path );


/*
 * UD_manifest_delete()
 * - free resources for a manifest instance.
 */
extern void          UD_manifest_delete( ud_manifest_t *man );

#endif /* UNUM_MANIFEST_H */
