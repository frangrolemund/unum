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

#include <assert.h>
#include <stdlib.h>

#include "u_common.h"
#include "u_fs.h"
#include "u_proc.h"

static uu_proc_t *proc_new( void );

#if UNUM_OS_UNIX
#include <errno.h>
static uu_proc_t *UU_proc_exec_unix( uu_cstring_t bin_name,
                                     uu_cstring_t *pargs, uu_cstring_t *penv,
                                     uu_proc_options_e opts, uu_error_e *err );
static pid_t     proc_waitpid(pid_t pid, int *stat_loc, int options);
#endif


uu_proc_t *UU_proc_exec( uu_cstring_t bin_name,  uu_cstring_t *pargs,
                         uu_cstring_t *penv, uu_proc_options_e opts,
                         uu_error_e *err ) {
#if UNUM_OS_UNIX
	return UU_proc_exec_unix(bin_name, pargs, penv, opts, err);
#else
#error "Not implemented"
#endif
}


#if UNUM_OS_UNIX
extern char **environ;
static uu_proc_t *UU_proc_exec_unix( uu_cstring_t bin_name,
                                     uu_cstring_t *pargs, uu_cstring_t *penv,
                                     uu_proc_options_e opts, uu_error_e *err ) {
	uu_proc_t    *ret        = NULL;
	uu_path_t    bpath;
	int          fdout[2]    = {0, 0};
	int          fderr[2]    = {0, 0};
	pid_t        child;
	uu_cstring_t *argv       = NULL;
	uu_string_t  *cur_env    = NULL;
	int          alen, elen;
	uu_string_t  buf;
	
	UU_set_errorp(err, UU_OK);
	
	if (!UU_path_normalize(bpath, bin_name, err)) {
		goto failed;
	}
	
	if (!(ret = proc_new())) {
		UU_set_errorp(err, UU_ERR_MEM);
		goto failed;
	}
	
	if ((opts & UU_PROC_CAPOUT) && (pipe(fdout) || pipe(fderr))) {
		UU_set_errorp(err, UU_ERR_FILE);
		goto failed;
	}

	alen = 1;
	for (int i = 0; pargs && pargs[i]; i++) {
		alen++;
	}
	
	ret->argv = argv = UU_mem_alloc(sizeof(uu_cstring_t) * (alen + 1));
	if (!argv) {
		UU_set_errorp(err, UU_ERR_MEM);
		goto failed;
	}
	
	for (int i = 0; i < alen; i++) {
		argv[i]   = UU_mem_strdup(i == 0 ? bpath : pargs[i-1]);
		if (!argv[i]) {
			UU_set_errorp(err, UU_ERR_MEM);
			goto failed;
		}
		argv[i+1] = NULL;
	}
	
	ret->argc = alen;
	
	child = fork();
	if (child < 0) {
		UU_set_errorp(err, UU_ERR_PROC);
		goto failed;
	}
	
	if (child > 0) {
		// - parent continuing...
		if (fdout[1]) close(fdout[1]);
		if (fderr[1]) close(fderr[1]);

		ret->pid   = child;
		ret->fdout = fdout[0];
		ret->fderr = fderr[0];
		fdout[0]   = fdout[1] = 0;
		fderr[0]   = fderr[1] = 0;
		
		if ((opts & UU_PROC_CAPOUT) &&
		    (!(ret->fstdout = fdopen(ret->fdout, "r")) ||
			 !(ret->fstderr = fdopen(ret->fderr, "r"))) ) {
			UU_set_errorp(err, UU_ERR_FILE);
			goto failed;
		}
		
		return ret;
	} else {
		// - child continuing...
		if ((opts & UU_PROC_CAPOUT)) {
			close(fdout[0]);
			close(fderr[0]);
			
			close(STDIN_FILENO);
			close(STDOUT_FILENO);
			close(STDERR_FILENO);
			
			if (dup2(fdout[1], STDOUT_FILENO) == -1 ||
				dup2(fderr[1], STDERR_FILENO) == -1) {
				exit(255);
			}
			
			close(fdout[1]);
			close(fderr[1]);
		}
				
		if (penv) {
			if (opts & UU_PROC_REPENV) {
				// - copy then modify
				elen = 0;
				for (char **tenv = environ; *tenv; tenv++) {
					cur_env         = realloc(cur_env,
					                          sizeof(uu_string_t) * (elen+1));
					cur_env[elen++] = strdup(*tenv);
					for (char *c = cur_env[elen-1]; *c; c++) {
						if (*c == '=') {
							*c = '\0';
							break;
						}
					}
				}
				for (int i = 0; i < elen; i++) {
					unsetenv(cur_env[i]);
				}
			}
			
			for (int i = 0; penv[i]; i++) {
				putenv((uu_string_t) penv[i]);
			}
		}
		
		if (execv(bpath, (char * const *) argv) == -1) {
			exit(253);
		}		
	}


failed:
	if (fdout[0]) close(fdout[0]);
	if (fdout[1]) close(fdout[1]);
	if (fderr[0]) close(fderr[0]);
	if (fderr[1]) close(fderr[1]);
	
	if (ret) {
		UU_proc_delete(ret);
	}

	return NULL;                              
}

static pid_t proc_waitpid(pid_t pid, int *stat_loc, int options) {
	pid_t ret;
	
	// ...during testing, there were occurrences of interruption
	//    that should be monitored.  If it continues into other
	//    syscalls, this approach may need to be formalized.
	
	if ((ret = waitpid(pid, stat_loc, options)) < 0 && errno == EINTR) {
		return waitpid(pid, stat_loc, options);
	}
	return ret;
}

#endif


static uu_proc_t *proc_new( void ) {
	uu_proc_t *ret = NULL;
	
	ret = (uu_proc_t *) UU_mem_alloc(sizeof(uu_proc_t));
	if (!ret) {
		return NULL;
	}

	UU_mem_set(ret, 0, sizeof(uu_proc_t));
	
#if UNUM_OS_UNIX
	ret->pid    = -1;
	ret->status = -1;
	ret->fdout  = -1;
	ret->fderr  = -1;
#endif

	return ret;
}


FILE *UU_proc_stdout( uu_proc_t *proc ) {
	return proc ? proc->fstdout : NULL;
}


FILE *UU_proc_stderr( uu_proc_t *proc ) {
	return proc ? proc->fstderr : NULL;
}


int UU_proc_wait( uu_proc_t *proc, uu_error_e *err ) {
	UU_set_errorp(err, UU_OK);
	
#if UNUM_OS_UNIX
	int stat_loc = 0;
	
	if (!proc || proc->pid < 1) {
		UU_set_errorp(err, UU_ERR_ARGS);
		return -1;
	}
	
	if (proc->status != -1) {
		return proc->status;
	}
	
	if (proc_waitpid(proc->pid, &stat_loc, WUNTRACED) < 1 ||
		WIFSIGNALED(stat_loc) || WIFSTOPPED(stat_loc)) {
		UU_set_errorp(err, UU_ERR_PROC);
		return -1;
	}
	
	proc->status = WEXITSTATUS(stat_loc);
	return proc->status;

#else
#error "Not implemented."
#endif
}


uu_error_e  UU_proc_kill( uu_proc_t *proc ) {
	return UU_ERR_NOIMPL;
}


void UU_proc_delete( uu_proc_t *proc ) {
	uu_cstring_t *p;
	
	UU_proc_kill(proc);
	UU_proc_wait(proc, NULL);
	
	if (proc) {
		if (proc->argv) {
			p = proc->argv;
			while (p && *p) {
				UU_mem_free((void *) *p++);
			}
			UU_mem_free(proc->argv);
		}
		
		if (proc->fstdout) {
			fclose(proc->fstdout);
		}
		
		if (proc->fstderr) {
			fclose(proc->fstderr);
		}
		
#if UNUM_OS_UNIX
		if (proc->fdout && !proc->fstdout) {
			close(proc->fdout);
		}
		
		if (proc->fderr && !proc->fstderr) {
			close(proc->fderr);
		}
#endif

		UU_mem_free(proc);
	}
}
