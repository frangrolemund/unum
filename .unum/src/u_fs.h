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

#define U_PATH_MAX       PATH_MAX
typedef char uu_path_t[U_PATH_MAX];

/*
 * UU_basename()
 * - copy the file name portion of a path from `src` into `dst`.
 */
extern uu_error_e   UU_basename( uu_string_t dst, size_t len,
                                 uu_cstring_t src );


/*
 * UU_dirname()
 * - copy the directory name portion of a path from `src` into `dst`.
 */
extern uu_error_e   UU_dirname( uu_string_t dst, size_t len, uu_cstring_t src );


/*
 * UU_file_info()
 * - retrieve file information or `st_mode==0` if failed.
 */
extern struct stat  UU_file_info( uu_cstring_t path );


/*
 * UU_is_file()
 * - identify if the provided path refers to a file.
 */
#define UU_is_file(p)   ((UU_file_info(p).st_mode & S_IFREG) ? true : false)


/*
 * UU_no_file()
 * - identify if the provided path does not refer to a valid file object.
 */
#define UU_no_file(p)   ((UU_file_info(p).st_mode == 0) ? true : false)


/*
 * UU_is_dir()
 * - identify if the provided path refers to a directory.
 */
#define UU_is_dir(p)    ((UU_file_info(p).st_mode & S_IFDIR) ? true : false)


/*
 * UU_mkdir()
 * - create a directory, including intermediaries as necessary.
 */
extern uu_error_e   UU_mkdir( uu_string_t dir, mode_t mode, uu_bool_t intermed);


/*
 * UU_path_join()
 * - join the path segments (NULL terminated, variable list uu_cstring_t at end)
 *   into a single path stored in `dst` and return the result.
 */
extern uu_cstring_t UU_path_join( uu_string_t dst, size_t len, ... );


/*
 * UU_path_join_s
 * - join the path segments (NULL terminated, variable list uu_cstring_t)
 *   into a single path stored in a static variable returned as the result.
 */
extern uu_cstring_t UU_path_join_s( uu_cstring_t item, ... );


/*
 * UU_path_prefix()
 * - return the first path segment from `path`, saving it in `dst`, returning
 *   successive levels of the hiearchy with subsequent calls until returning
 *   NULL when there are no more sub-items.
 */
extern uu_cstring_t UU_path_prefix( uu_string_t dst, size_t len,
                                    uu_cstring_t path );


/*
 * UU_realpath()
 * - resolves all symlinks and extra path characters, returning the result
 *   in `dst`, which must be able to hold a path.
 */
extern uu_cstring_t UU_realpath( uu_string_t dst, uu_cstring_t path, 
                                 uu_error_e *err );


#endif /* UNUM_FS_H */
