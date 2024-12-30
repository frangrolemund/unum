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

#include <stdarg.h>

#include "u_common.h"
#include "u_fs.h"

typedef char * uu_string_t;

static uu_string_t path_vjoin( uu_string_t dst, size_t len, uu_string_t dstp,
                               va_list ap);

uu_error_e UU_path_basename( uu_string_t dst, size_t len, uu_cstring_t src ) {
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


uu_error_e UU_path_dirname( uu_string_t dst, size_t len, uu_cstring_t src ) {
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
	
	UU_mem_set(&s, 0, sizeof(s));
	return s;	
}


uu_error_e UU_dir_create( uu_string_t dir, mode_t mode, uu_bool_t intermed) {
	uu_path_t    state = {'\0'};
	uu_cstring_t cur;
	struct stat  s;
	
	while ((cur = UU_path_prefix(state, U_PATH_MAX, dir))) {
		s = UU_file_info(cur);
		
		if (s.st_mode & S_IFDIR) {
			continue;

		} else if (s.st_mode != 0 || !intermed || mkdir(cur, mode)) {
			return UU_ERR_FILE;
		}	
	}
	
	return UU_OK;
}


uu_cstring_t UU_path_join( uu_string_t dst, size_t len, ...) {
	va_list      ap;
	
	va_start(ap, len);
	dst = path_vjoin(dst, len, dst, ap);
	va_end(ap);
	
	return dst;
}


extern uu_cstring_t UU_path_join_s( uu_cstring_t item, ... ) {
	static uu_path_t buf   = {'\0'};
	size_t           ilen;
	uu_string_t      ret;
	va_list			 ap;
	
	
	if (!item || (ilen = strlen(item)) >= U_PATH_MAX) {
		return NULL;
	}
	
	strcpy(buf, item);
	va_start(ap, item);
	ret = path_vjoin(buf, U_PATH_MAX - ilen, buf + ilen, ap);
	va_end(ap);
	
	return ret;
}


static uu_string_t path_vjoin( uu_string_t dst, size_t len, uu_string_t dstp,
                               va_list ap) {
	uu_cstring_t src;

	while ((src = va_arg(ap, uu_cstring_t))) {

		if (len && dstp > dst && *(dstp - 1) != UNUM_PATH_SEP) {
			*dstp++ = UNUM_PATH_SEP;
			len--;
		}

		for (;;) {
			if (!len) {
				goto join_done;
			}
					
			if (!(*dstp = *src)) {
				break;
			}
			
			src++;
			dstp++;
			len--;
		}
	}


join_done:

	if (len) {
		*dstp = '\0';
		
	} else {
		dst   = NULL;
	}
	
	return dst;
}


extern uu_cstring_t UU_path_prefix( uu_string_t dst, size_t len,
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


uu_cstring_t UU_path_normalize( uu_string_t dst, uu_cstring_t path,
                                uu_error_e *err ) {
	uu_path_t ret;  // - to support dst == path
	
	UU_set_errorp(err, UU_OK);
	
#if UNUM_OS_MACOS
	if (!dst || !realpath(path, ret)) {
		UU_set_errorp(err, dst ? UU_ERR_FILE : UU_ERR_ARGS);
		return NULL;
	}
	
	strcpy(dst, ret);
	return dst;
	
#else
	#error "Not implemented."

#endif
}


extern uu_cstring_t UU_path_normalize_s( uu_cstring_t path, uu_error_e *err ) {
	static uu_path_t ret = { '\0' };
	
	return UU_path_normalize(ret, path, err);
}
