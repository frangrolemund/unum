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

#include "u_common.h"
#include "u_fs.h"

typedef char * uu_string_t;

uu_error_e UU_basename( uu_string_t dst, uu_cstring_t src, size_t len ) {
	int blen = 0;
	
	while (src && *src) {
		src++;
	}
	
	while (src && *--src != UNUM_PATH_SEP) {
		blen++;
	}
	
	if (!blen || len < blen) {
		return UU_ERR_ARGS;
	}
	
	do {
		*dst++ = *++src;
	} while (blen--);
	
	return UU_OK;
}


uu_error_e UU_dirname( uu_string_t dst, uu_cstring_t src, size_t len ) {
	size_t i   = 0;
	char *last = dst;
	
	if (!dst || !src || !*src) {
		return UU_ERR_ARGS;
	}
	
	for (; *src; src++, i++) {
		if (i >= len) {
			return UU_ERR_ARGS;
		}

		if ((dst[i] = *src) == UNUM_PATH_SEP) {
			last = &dst[i];
		}
	}
	
	if (last == dst) {
		*last   = '\0';
	} else {
		*++last = '\0';  // - include trailing path delim
	}

	return UU_OK;
}


struct stat UU_file_info( uu_cstring_t path ) {
	struct stat s;
	
	if (path && *path && stat(path, &s) == 0) {
		return s;
	}
	
	UU_memset(&s, 0, sizeof(s));
	return s;	
}


extern uu_cstring_t UU_pop_seg( uu_cstring_t path, uu_string_t state,
                                size_t len, uu_error_e *err ) {
	uu_cstring_t ret = state;
	
	UU_set_errorp(err, UU_OK);
                                
	while (*state == *path && len > 0 && *path) {
		path++;
		state++;
		len--;
	}
		
	do {
		if (!len || !*path) {
			UU_set_errorp(err, len ? UU_OK : UU_ERR_ARGS);
			return NULL;
		}
	
		*state++ = *path;
		if (*path == UNUM_PATH_SEP) {
			break;
		}
		
		len--;
	} while (*++path);
	*state = '\0';
	
	return (ret != state) ? ret : NULL;
}


uu_cstring_t UU_realpath( uu_cstring_t path, uu_string_t state,
                          uu_error_e *err ) {
	static uu_path_t ret;
	char             *buf = state ? state : ret;
	
	if (!state) {
		UU_set_errorp(err, UU_ERR_ARGS);
		return NULL;
	}
	
	UU_set_errorp(err, UU_OK);
	
#if UNUM_OS_MACOS
	if (!realpath(path, buf)) {
		UU_set_errorp(err, UU_ERR_FILE);
		return NULL;
	}
	
	return buf;
	
#else
	#error "Not implemented."

#endif
}
