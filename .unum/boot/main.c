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

/*
 *  The purpose of bootstrapping is to ingest the C compiler environment
 *  from a platform-specific toolchain (ie. `make`) into the 
 *  platform-independent `unum` toolchain to allow the latter take over all
 *  further build processing.  
 *
 *  This program figures out where it is running, encodes fundamental 
 *  configuration and builds the first `unum` kernel, called the pre-kernel
 *  or 'pre-k' for short.  The objective is for it to be as minimal as possible
 *  without confusing the process or introducing needless waste when `make` is
 *  is re-invoked on an existing repo.
 */

#include <assert.h>
#include <limits.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

// - types
typedef enum {
	T_CC = 0,
	T_LD,

	T_COUNT
} tool_e;

static const struct { tool_e tool; const char *oname; } tool_map[] = {
	{ T_CC, "cc" },
	{ T_LD, "ld" }
};

typedef enum {
	P_WINDOWS = 0,
	P_MACOS,
	P_LINUX,
	P_UNKNOWN	
} platform_e;

// - forward declarations
static void abort_fail(const char *fmt, ...);
static void detect_path_style();
static void detect_platform();
static struct stat file_info(const char *path);
static char *find_in_path(const char *cmd);
static int run_cc(const char *code);
static const char *parse_option(const char *optName, const char *from);
static void parse_cmd_line(int argc, char *argv[]);
static const char *resolve_cmd(const char *cmd);
static void set_basis();
static const char *to_basis(const char *path);

// - state
static char        basis_dir[PATH_MAX];
static char        path_sep;
static platform_e  platform        = P_UNKNOWN;
static const char  *tools[T_COUNT] = { NULL, NULL };
static FILE        *uberr          = NULL;

#define BUILD_DIR 	       "./build"
#define BUILD_INCLUDE_DIR  "./build/include"
#define BIN_DIR            "./bin"
#define is_file(p)         (file_info((p)).st_mode & S_IFREG)
#define is_dir(p)          (file_info((p)).st_mode & S_IFDIR)
#define path_sep_s         ((char [2]) { path_sep, '\0' })


int main(int argc, char *argv[]) {
	// - don't spam stderr with system()
	uberr = fdopen(dup(fileno(stderr)), "w");
	fclose(stderr);

	// - order is important here
	detect_path_style();
	parse_cmd_line(argc, argv);
	detect_platform();

	set_basis();
	

	printf("inside uboot...\n");
	printf("- basis:          %s\n", basis_dir);
	printf("- path separator: '%c'\n", path_sep);
	printf("- platform:  	   %d\n", platform);
	for (int i = 0; i < T_COUNT; i++) {
		printf("- tool %d:         %s\n", i, tools[i]);
	}

	fclose(uberr);
	return 0;
}


static void abort_fail(const char *fmt, ...) {
	va_list ap;
	char    buf[2048];

	va_start(ap, fmt);
	vsnprintf(buf, sizeof(buf), fmt, ap);
	va_end(ap);

	fprintf(uberr ? uberr : stdout, "uboot error: %s\n", buf);
	exit(1);
}


static void detect_path_style() {
	const char  *path = getenv("PATH");
	char        c;

	while (path && (c = *path++)) {
		if (c == '/' || c == '\\') {
			path_sep = c;
			return;
		}
	}

	abort_fail("missing PATH environment");
}


static const char *parse_option(const char *opt_name, const char *from) {
	char       prefix[64] = "\0";
	const char *p         = prefix;

	snprintf(prefix, sizeof(prefix), "--%s=", opt_name);

	while (*p) {
		if (*p++ != *from++) {
			return NULL;
		}
	}

	return from;
}


static struct stat file_info(const char *path) {
	struct stat sinfo;

	if (path && stat(path, &sinfo) == 0) {
		return sinfo;
	}

	memset(&sinfo, 0, sizeof(sinfo));
	return sinfo;
}


static char *find_in_path(const char *cmd) {
	const char   *path = getenv("PATH");
	static char  buf[PATH_MAX];
	char         *ppos = buf;
	char         c;

	while ((c = *path++)) {
		if (c == ':' || c == ';') {
			*ppos++ = path_sep;
			strcpy(ppos, cmd);

			if (is_file(buf)) {
				return buf;
			}

			ppos = buf;
		} else {
			*ppos++ = c;
		}
	}

	return NULL;
}

static const char *resolve_cmd(const char *cmd) {
	char tmp[PATH_MAX];
	const char *rc     = NULL;

	if (!cmd || !cmd[0]) {
		return NULL;
	}

	const int has_dir = strstr(cmd, path_sep_s) != NULL;
	const int is_rel  = cmd[0] == '.';

	// - the principle here is to use as little and make this as
	//   simple as possible to  establish that the path exists while 
	//	 assuming the kernel will more carefully assess behavior later.  
	// 	 Ooddly-formed paths are permitted, (eg. /usr/bin/../bin/cc)
	if (is_rel) {
		getcwd(tmp, PATH_MAX);
		strcat(tmp, path_sep_s);
		strcat(tmp, cmd);
		rc = tmp;
	} else if (has_dir) {
		rc = cmd;
	} else {
		rc = find_in_path(cmd);	
	}

	if (!rc || !is_file(rc) || (rc = strdup(rc)) == NULL) {
		abort_fail("unresolvable command path '%s'", cmd);
	}

	return rc;
}


static void parse_cmd_line(int argc, char *argv[]) {
	const char  *opt = NULL;
	int         i;

	while (--argc) {
		const char *item = *++argv;
		int is_sup = 0;
		for (i = 0; i < sizeof(tool_map)/sizeof(tool_map[0]); i++) {
			if ((opt = parse_option(tool_map[i].oname, item))) {
				opt = (opt && opt[0]) ? opt : NULL;
				opt = (i == T_CC || i == T_LD) ? resolve_cmd(opt) : opt;
				tools[tool_map[i].tool] = opt;
				is_sup = 1;
				break;
			}	
		}

		if (!is_sup) {
			abort_fail("unsuported command-line parameter '%s'", item);
		}
	}

	if (!tools[T_CC] || !tools[T_LD]) {
		abort_fail("missing one or more required tool parameters.");
	}
}


static const char *to_basis(const char *path) {
	static char  buf[PATH_MAX];
	char         *bp = buf, *tmp;

	for (tmp = basis_dir; *tmp; tmp++, bp++) {
		*bp = (*tmp == '/') ? path_sep : *tmp;
	}

	for (; *path; path++, bp++) {
		*bp = *path;
	}

	*bp = '\0';
	
	return buf;
}


static void set_basis() {
	char        cwd[PATH_MAX];
	char        *pos;
	const char  *bd;
	const char  *build_dirs[] = { BUILD_DIR, BUILD_INCLUDE_DIR, BIN_DIR };
	int         i;

	if (!getcwd(cwd, PATH_MAX)) {
		abort_fail("cannot retrieve cwd");
	}
	
	if (strncmp(cwd, __FILE__, strlen(cwd))) {
		snprintf(basis_dir, sizeof(basis_dir), "%s%c%s", cwd, path_sep, 
		         __FILE__);
	} else {
		strncpy(basis_dir, __FILE__, sizeof(basis_dir));
	}

	pos = basis_dir + strlen(basis_dir);
	while (--pos >= basis_dir) {
		if (!strncmp(pos, "boot", 4)) {
			*pos = '\0';
			break;
		}
	}

	if (pos <= basis_dir) {
		abort_fail("basis not found");
	}

	chdir(basis_dir);

	for (i = 0; i < sizeof(build_dirs)/sizeof(build_dirs[0]); i++) {
		bd = to_basis(build_dirs[i]);
		if (!is_dir(bd) && mkdir(bd, S_IRWXU) != 0) {
			abort_fail("failed to create build directory '%s'", bd);
		}
	}
}


static int run_cc(const char *source) {
	const char *tmp_env[]         = { "TMPDIR", "TMP", "TEMP", "TEMPDIR", 
	                                  NULL };
	const char **tp               = tmp_env;
	int        rc;
	char       src_name[PATH_MAX];
	char       bin_name[PATH_MAX];
	char       cc_cmd[2048];
	FILE       *src_file;

	// ...avoids the scary warning from macos for using tmpnam
	do {
		const char *e_val = getenv(*tp);

		if (e_val) {
			snprintf(src_name, PATH_MAX, "%s%cunum-boot.c", e_val, path_sep);
			break;	
		}

		if (!*++tp) {
			abort_fail("failed to find temp directory!.");
		}
	} while (*tp);

	snprintf(bin_name, PATH_MAX, "%s.out", src_name);

	src_file = fopen(src_name, "w");
	if (!src_file || !fwrite(source, strlen(source), 1, src_file)) {
		abort_fail("failed to create temp file.");
	}
	fclose(src_file);

	snprintf(cc_cmd, sizeof(cc_cmd), "%s -o %s %s", 
				tools[T_CC], bin_name, src_name);

	rc = system(cc_cmd);

	unlink(src_name);
	unlink(bin_name);

	return rc;
}


static void detect_platform() {
	if (run_cc(
	           "#include <Carbon/Carbon.h>\n"	
	           "#include <stdio.h>\n\n"
	           "int main(int argc, char **argv) {\n"
	           "  printf(\"hello unum\");\n"
	           "}\n"
	          ) == 0) {
		platform = P_MACOS;
		return;
	}

	abort_fail("unsupported platform type");
}
