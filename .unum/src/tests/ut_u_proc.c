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
#include <unistd.h>

#include "u_proc.h"
#include "u_test.h"


static uu_cstring_t prog;

static int          unittest_proc( int argc, char *argv[] );
static void         proc_test_bad( void );
static uu_proc_t    *t_proc_exec(uu_cstring_t cmd, uu_cstring_t *args,
                                 uu_cstring_t *env, uu_bool_t read_out,
                                 uu_error_e *err);
static uu_cstring_t t_proc_stdread( uu_proc_t *proc );
static int          selftest_run( uu_cstring_t arg_selftest );

#define ARG_SELFTEST "--selftest="
#define CMD_OKRC     "okrc"
#define CMD_BADRC    "badrc"

int main( int argc, char *argv[] ) {
	uu_cstring_t a_selftest = NULL;
	size_t       len_st     = strlen(ARG_SELFTEST);
	for (int i = 0; i < argc; i++) {
		if (!strncmp(argv[i], ARG_SELFTEST, len_st)) {
			a_selftest = argv[i] + len_st;
			break;
		}
	}
	
	// - this test is its own child-process
	if (a_selftest) {
		return selftest_run(a_selftest);
	
	} else {
		prog = argv[0];
		return UT_test_run(argc, argv, unittest_proc);
	}
}


static int unittest_proc( int argc, char *argv[] ) {
	proc_test_bad();
	
	return 0;
}


static void proc_test_bad( void ) {
	uu_proc_t    *proc;
	uu_error_e   err;

	UT_test_setname("ok-bad test");
	
	proc = t_proc_exec(CMD_BADRC, NULL, NULL, false, &err);
	UT_test_assert(proc && err == UU_OK, "Failed to get process.");
	UT_test_assert(UU_proc_wait(proc, &err) == 3 && err == UU_OK,
	               "failed to get result.");
	UU_proc_delete(proc);
	
	proc = t_proc_exec(CMD_BADRC, NULL, NULL, true, &err);
	UT_test_assert(proc && err == UU_OK, "Failed to get process.");
	UT_test_assert(UU_proc_stdout(proc), "Failed to get standard output.");
	UT_test_assert(UU_proc_stderr(proc), "Failed to get standard error.");
	UT_test_assert_eq(t_proc_stdread(proc),
	                  "/* stdout */\nfake_attempt()\n"
					  "/* stderr */\nut_u_proc: planned fail\n",
	                  "Failed to get standard result.");
	UU_proc_delete(proc);
}


static uu_proc_t *t_proc_exec(uu_cstring_t cmd, uu_cstring_t *args,
                              uu_cstring_t *env, uu_bool_t read_out,
                              uu_error_e *err) {
	uu_cstring_t t_args[32];
	char         st_arg[128];

	sprintf(st_arg, "%s%s", ARG_SELFTEST, cmd);
	t_args[0] = st_arg;
	t_args[1] = NULL;
	for (int i = 1; args; args++, i++) {
		t_args[i] = *args;
		if (!*args) {
			break;
		}
	}

	return UU_proc_exec(prog, t_args, env, read_out, err);
}


static uu_cstring_t t_proc_stdread( uu_proc_t *proc ) {
	static uu_string_t buf      = NULL;
	static size_t      blen     = 0;
	size_t             offset   = 0;
	char               tmp[256];
	size_t             num_read = 0;
	uu_bool_t          has_err  = false;

	UT_test_assert(UU_proc_stdout(proc) && UU_proc_stderr(proc),
                   "failed to get standard handles.");

	
	while (!feof(UU_proc_stdout(proc)) && !ferror(UU_proc_stdout(proc))) {
		if ((num_read = fread(tmp, 1, sizeof(tmp), UU_proc_stdout(proc)))) {
			if (!buf) {
				UT_test_assert(buf = UU_mem_restrcat(buf, "/* stdout */\n",
				               &blen), "out of memory.");
			}

			tmp[num_read] = '\0';
			UT_test_assert(buf = UU_mem_restrcat(buf, tmp, &blen),
			               "out of memory.");
			offset += num_read;
		}
	}
	
	while (!feof(UU_proc_stderr(proc)) && !ferror(UU_proc_stderr(proc))) {
		if ((num_read = fread(tmp, 1, sizeof(tmp), UU_proc_stderr(proc)))) {
			if (!has_err) {
				UT_test_assert(buf = UU_mem_restrcat(buf, "/* stderr */\n",
				               &blen), "out of memory.");
				has_err = true;
			}
			
			tmp[num_read] = '\0';
			UT_test_assert(buf = UU_mem_restrcat(buf, tmp, &blen),
			               "out of memory");
			offset += num_read;
		}
	}
	
	return buf;
}

/*
 *  -- SELF TEST --
 */
static int selftest_run( uu_cstring_t arg_selftest ) {
	if (!strcmp(arg_selftest, CMD_OKRC)) {
		fprintf(stdout, "ut_u_proc: success\n");
		return 0;
		
	} else if (!strcmp(arg_selftest, CMD_BADRC)) {
		fprintf(stdout, "fake_attempt()\n");
		// ...imagine failure occurs.
		fprintf(stderr, "ut_u_proc: planned fail\n");
		return 3;
	}

	fprintf(stderr, "ut_u_proc: unsupported self-test '%s'\n", arg_selftest);
	return -255;
}
