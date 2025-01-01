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

#include "d_manifest.h"

/*
 *  Systemic manifest:
 *  - accounting for all project source files in platform-independent format
 *  - supports manual editing where desired
 *  - auto-reformatting for format upgrades or hand-editing with external tools
 *  - simple access patterns to ensure correctness
 */


static ud_manifest_t       *manifest_new( uu_cstring_t root, uu_error_e *err );
static uu_bool_t           manifest_reload( ud_manifest_t *man,
                                            uu_cstring_t path,
                                            uu_error_e *err );
static uu_cstring_t        phase_to_text( ud_manifest_phase_e phase );
static ud_manifest_phase_e text_to_phase( uu_cstring_t text );


typedef enum {
	UD_MANC_FILE  = 0,
	UD_MANC_PHASE,
	UD_MANC_REQ,
	UD_MANC_NAME,
	
	UD_MANC_COUNT
} ud_manifest_column_e;


#define COL_FILE        "file"
#define COL_PHASE       "phase"
#define COL_REQ         "requires"
#define COL_NAME        "name"

#define PHASE_CORE      "core"
#define PHASE_KERN      "kernel"
#define PHASE_CUSTOM    "custom"
#define PHASE_TEST      "test"

#define UD_MANP_INVALID -1


ud_manifest_t *UD_manifest_new( uu_cstring_t root, uu_error_e *err ) {
	ud_manifest_t *ret;
	
	if (!(ret = manifest_new(root, err))) {
		return NULL;
	}
	
	if (!(ret->csv = UU_csv_new(UD_MANC_COUNT)) ||
	    !UU_csv_add_row(ret->csv) ||
		UU_csv_set(ret->csv, 0, 0, COL_FILE) != UU_OK ||
		UU_csv_set(ret->csv, 0, 1, COL_PHASE) != UU_OK ||
		UU_csv_set(ret->csv, 0, 2, COL_REQ) != UU_OK ||
		UU_csv_set(ret->csv, 0, 3, COL_NAME) != UU_OK) {
		UD_manifest_delete(ret);
		UU_set_errorp(err, UU_ERR_MEM);
		return NULL;
	}
	
	return ret;
}


ud_manifest_t *UD_manifest_open( uu_cstring_t root, uu_cstring_t path,
                                 uu_error_e *err ) {
	return NULL;
}


static uu_bool_t manifest_reload( ud_manifest_t *man, uu_cstring_t path,
                                  uu_error_e *err ) {
	uu_csv_t     *cf;
	uu_cstring_t from_path;
	
	UU_set_errorp(err, UU_OK);
	
	if (!man || !man->csv ||
	    (!(from_path = UU_csv_file_path(man->csv)) && !(from_path = path))) {
		UU_set_errorp(err, UU_ERR_ARGS);
		return false;
	}
	
	if (!(cf = UU_csv_open(from_path, err))) {
		return false;
	}
	
	// TODO: validate/remap
	
	// TODO: reassign
	
	return true;
}


static ud_manifest_t *manifest_new( uu_cstring_t root, uu_error_e *err ) {
	ud_manifest_t *ret;
	uu_path_t     root_res;
	
	UU_set_errorp(err, UU_OK);
	
	if (!UU_path_normalize(root_res, root, err)) {
		return NULL;
	}
		
	ret = UU_mem_alloc(sizeof(ud_manifest_t));
	if (!ret) {
		UU_set_errorp(err, UU_ERR_MEM);
		return NULL;
	}

	UU_mem_reset(ret, sizeof(ud_manifest_t));
	strcpy(ret->root, root_res);
	ret->csv = NULL;
	
	return ret;
}


void UD_manifest_delete( ud_manifest_t *man ) {
	if (!man) {
		return;
	}
	
	if (man->csv) {
		UU_csv_delete(man->csv);
	}
	
	UU_mem_free(man);
}


uu_error_e UD_manifest_add_file( ud_manifest_t *man, ud_manifest_file_t file ) {
	uu_error_e   err;
	int          row    = -1;
	uu_cstring_t value, ptext, rtext;
	
	if (!man || !man->csv) {
		return UU_ERR_ARGS;
	}
	
	if (UU_path_normalize(file.res1, file.path, &err) == NULL) {
		return err;
	}
	
	if (strstr(file.res1, man->root) != file.res1 ||
	   (file.name && strlen(file.name) >= U_MANIFEST_MAX_NAME) ||
	   (file.req > file.phase) ||
	   !(ptext = phase_to_text(file.phase)) ||
	   !(rtext = phase_to_text(file.req))) {
		return UU_ERR_ARGS;
	}
	
	if (!UU_file_exists(file.res1)) {
		return UU_ERR_FILE;
	}

	file.path = file.res1 + strlen(man->root) + 1;
	if (!UU_path_to_independent(file.res1, U_PATH_MAX, file.path)) {
		return UU_ERR_ARGS;
	}
	
	for (int i = 0; i < UU_csv_row_count(man->csv); i++) {
		if ((value = UU_csv_get(man->csv, i, UD_MANC_FILE, NULL)) &&
		    !strcmp(value, file.res1)) {
			row = i;
			break;
		}
	}
	
	if (row == -1) {
		row = (int) UU_csv_add_row(man->csv);
		if (!row) {
			return UU_ERR_MEM;
		}
		row--;
	}
	
	if (UU_csv_set(man->csv, (unsigned) row, UD_MANC_FILE, file.res1) ||
		UU_csv_set(man->csv, (unsigned) row, UD_MANC_PHASE, ptext) ||
		UU_csv_set(man->csv, (unsigned) row, UD_MANC_REQ, rtext) ||
		UU_csv_set(man->csv, (unsigned) row, UD_MANC_NAME, file.name)) {
		return UU_ERR_MEM;
	}

	return UU_OK;
}


static uu_cstring_t phase_to_text( ud_manifest_phase_e phase ) {
	switch (phase) {
	case UD_MANP_CORE:
		return PHASE_CORE;
		break;
			
	case UD_MANP_KERN:
		return PHASE_KERN;
		break;
			
	case UD_MANP_CUSTOM:
		return PHASE_CUSTOM;
		break;
			
	case UD_MANP_TEST:
		return PHASE_TEST;
		break;
			
	default:
		// invalid phase
		UU_assert(0);
		return NULL;
		break;
	}
}


static ud_manifest_phase_e text_to_phase( uu_cstring_t text ) {
	if (!strcmp(text, PHASE_CORE)) {
		return UD_MANP_CORE;

	} else if (!strcmp(text, PHASE_KERN)) {
		return UD_MANP_KERN;

	} else if (!strcmp(text, PHASE_CUSTOM)) {
		return UD_MANP_CUSTOM;

	} else if (!strcmp(text, PHASE_TEST)) {
		return UD_MANP_TEST;
	}

	return UD_MANP_INVALID;
}


unsigned UD_manifest_file_count( ud_manifest_t *man ) {
	unsigned ret = 0;
	
	if (man && man->csv && (ret = UU_csv_row_count(man->csv)) && ret > 0) {
		return ret - 1;
	}

	return 0;
}


uu_bool_t UD_manifest_get( ud_manifest_t *man, unsigned index,
                           ud_manifest_file_t *file ) {
	return false;
}


uu_error_e UD_manifest_delete_file ( ud_manifest_t *man, uu_cstring_t path ) {
	return UU_ERR_NOIMPL;
}


uu_error_e UD_manifest_delete_file_n ( ud_manifest_t *man, unsigned index ) {
	return UU_ERR_NOIMPL;
}


extern uu_cstring_t UD_manifest_root( ud_manifest_t *man ) {
	if (!man || !man->csv) {
		return NULL;
	}
	return man->root;
}


uu_error_e UD_manifest_write( ud_manifest_t *man, uu_cstring_t path ) {
	if (!man || !man->csv) {
		return UU_ERR_ARGS;
	}
	
	// TODO: reorganize when order differs
	// TODO: atomic write
	
	return UU_csv_write(man->csv, path);
}
