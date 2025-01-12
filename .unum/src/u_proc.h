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

#ifndef UNUM_PROC_H
#define UNUM_PROC_H

#include <stdio.h>
#if UNUM_OS_UNIX
#include <unistd.h>
#endif

#include "u_common.h"

/*
 *  Process management:
 *  - invoke executables with command-line
 *  - interact with sub-process stdio
 *  - kill or wait w/return code
 */


typedef struct {
	int          argc;
	uu_cstring_t *argv;
	FILE         *fstdout;
	FILE         *fstderr;
	
	#if UNUM_OS_UNIX
	pid_t        pid;
	int          status;
	int          fdout;
	int          fderr;
		
	#else
	#error "Not implemented."
	#endif

} uu_proc_t;


/*
 * UU_proc_exec()
 * - start a new process indicated by the program `bin_name`, optionally
 *   providing arguments (as `args`) (following the first), environment
 *   (as `env`) and any error generated.  If `read_out` is `true`, standard
 *   output/error may be read later, otherwise they are shared with the
 *   initiating process.  NULLs may be passed for optional parameters, but if
 *   arrays of strings are passed they *MUST* be NULL-terminated. Returns a
 *   process handle or NULL when an error occurs.
 */
extern uu_proc_t   *UU_proc_exec( uu_cstring_t bin_name,  uu_cstring_t *pargs,
								  uu_cstring_t *penv, uu_bool_t read_out,
                                  uu_error_e *err );
                                  

/*
 * UU_proc_delete()
 * - delete the process handle, killing the process if it is still running.
 */
extern void        UU_proc_delete( uu_proc_t *proc );


/*
 * UU_proc_kill()
 * - kill the process, returning an error if one occurs.
 */
extern uu_error_e  UU_proc_kill( uu_proc_t *proc );


/*
 * UU_proc_stderr()
 * - return a standard error file from the process or NULL if an error occurs.
 *   The returned file should not be released by the caller.
 */
extern FILE        *UU_proc_stderr( uu_proc_t *proc );


/*
 * UU_proc_stdout()
 * - return a standard output file from the process or NULL if an error occurs.
 *   The returned file should not be released by the caller.
 */
extern FILE        *UU_proc_stdout( uu_proc_t *proc );


/*
 * UU_proc_wait()
 * - waits on the process to complete naturally, returning its exit status or
 *   -1 in the event of an error.
 */
extern int         UU_proc_wait( uu_proc_t *proc, uu_error_e *err );


#endif /* UNUM_PROC_H */
