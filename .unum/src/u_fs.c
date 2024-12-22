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

uu_error_e UU_basename( uu_string_t dst, size_t len, uu_cstring_t src ) {
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


uu_error_e UU_dirname( uu_string_t dst, size_t len, uu_cstring_t src ) {
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


uu_cstring_t UU_path_join( uu_string_t dst, size_t len, uu_cstring_t segs[] ) {
	uu_cstring_t *cur = segs;
	uu_cstring_t src;
	char         last = 0;

	dst[0] = '\0';
	
	while ((src = *cur)) {
		for (;;) {
			if (!len) {
				return NULL;
			}

			*dst = *src;
			len--;

			if (!*src) {
				break;
			
			
			}
		}

		cur++;
	}
	
	return dst;
}


extern uu_cstring_t UU_path_pop( uu_string_t dst, size_t len,
                                 uu_cstring_t path ) {
	uu_cstring_t ret = dst;
	
	while (*dst == *path && len > 0 && *path) {
		path++;
		dst++;
		len--;
	}
		
	do {
		if (!len || !*path) {
			return NULL;
		}
	
		*dst++ = *path;
		if (*path == UNUM_PATH_SEP) {
			break;
		}
		
		len--;
	} while (*++path);
	*dst = '\0';
	
	return (ret != dst) ? ret : NULL;
}


uu_cstring_t UU_realpath( uu_string_t dst, uu_cstring_t path,
                          uu_error_e *err ) {
	UU_set_errorp(err, UU_OK);
	
#if UNUM_OS_MACOS
	if (!dst || !realpath(path, dst)) {
		UU_set_errorp(err, dst ? UU_ERR_FILE : UU_ERR_ARGS);
		return NULL;
	}
	
	return dst;
	
#else
	#error "Not implemented."

#endif
}
