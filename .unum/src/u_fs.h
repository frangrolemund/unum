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

#ifndef UNUM_FS_H
#define UNUM_FS_H

#include <sys/stat.h>

#include "u_common.h"

#define U_PATH_MAX       2048

/*
 * UU_basename()
 * - copy the file name portion of a path from `src` into `dst`.
 */
extern uu_error_e UU_basename( char *dst, const char *src, size_t len );


/*
 * UU_dirname()
 * - copy the directory name portion of a path from `src` into `dst`.
 */
extern uu_error_e UU_dirname( char *dst, const char *src, size_t len );


/*
 * UU_file_info()
 * - retrieve file information or `st_mode==0` if failed.
 */
extern struct stat UU_file_info( const char *path );

/*
 * UU_is_file()
 * - identify if the provided path refers to a file.
 */
#define UU_is_file(p)   (UU_file_info(p).st_mode & S_IFREG)


#endif /* UNUM_FS_H */
