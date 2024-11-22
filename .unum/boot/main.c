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
 *  without confusing the process or introducing needless waste if/when `make` 
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

typedef const char * cstrarr_t[];


static void abort_fail(const char *fmt, ...);
static void detect_path_style();
static void detect_platform();
static struct stat file_info(const char *path);
static char *find_in_path(const char *cmd);
static int run_cc(const char *bin_file, const char *include_dirs[], 
                  const char *source_files[]);
static int run_cc_with_source(const char *source); 
static const char *parse_option(const char *optName, const char *from);
static void parse_cmd_line(int argc, char *argv[]);
static void printf_config(const char *fmt, ...);
static const char *resolve_cmd(const char *cmd);
static void set_basis();
static const char *to_basis(const char *path);
static void write_config();


#define BUILD_DIR 	       "./build"
#define BUILD_INCLUDE_DIR  "./build/include"
#define BIN_DIR            "./bin"
#define is_file(p)         (file_info((p)).st_mode & S_IFREG)
#define is_dir(p)          (file_info((p)).st_mode & S_IFDIR)
#define path_sep_s         ((char [2]) { path_sep, '\0' })
#define CFG_SIZE           32768


static char        basis_dir[PATH_MAX];
static char        path_sep;
static platform_e  platform           = P_UNKNOWN;
static const char  *tools[T_COUNT]    = { NULL, NULL };
static FILE        *uberr             = NULL;
static char        config[CFG_SIZE];
static char        *cfg_offset        = config;


int main(int argc, char *argv[]) {
	// - don't spam stderr with system()
	uberr = fdopen(dup(fileno(stderr)), "w");
	fclose(stderr);

	// - order is important here
	detect_path_style();
	parse_cmd_line(argc, argv);
	detect_platform();

	set_basis();
	write_config();	

	printf("DEBUG: inside uboot...\n");
	printf("- basis:          %s\n", basis_dir);
	printf("- path separator: '%c'\n", path_sep);
	printf("- platform:  	   %d\n", platform);
	for (int i = 0; i < T_COUNT; i++) {
		printf("- tool %d:         %s\n", i, tools[i]);
	}

	fclose(uberr);
	return 0;
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


static void abort_fail(const char *fmt, ...) {
	va_list ap;
	char    buf[2048];

	va_start(ap, fmt);
	vsnprintf(buf, sizeof(buf), fmt, ap);
	va_end(ap);

	fprintf(uberr ? uberr : stdout, "uboot error: %s\n", buf);
	exit(1);
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


static struct stat file_info(const char *path) {
	struct stat sinfo;

	if (path && stat(path, &sinfo) == 0) {
		return sinfo;
	}

	memset(&sinfo, 0, sizeof(sinfo));
	return sinfo;
}


static void detect_platform() {
	if (run_cc_with_source(
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


static int run_cc_with_source(const char *source) {
	const char *tmp_env[]         = { "TMPDIR", "TMP", "TEMP", "TEMPDIR", 
	                                  NULL };
	const char **tp               = tmp_env;
	int        rc;
	char       src_name[PATH_MAX];
	char       bin_name[PATH_MAX];
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

	rc = run_cc(bin_name, (cstrarr_t) { NULL }, (cstrarr_t) { src_name, NULL });

	unlink(src_name);
	unlink(bin_name);

	return rc;
}


// - both include_dirs and source_files must be terminated with NULL
static int run_cc(const char *bin_file, cstrarr_t include_dirs, 
                  cstrarr_t source_files) {
	char cmd[8192];
	
	strncpy(cmd, tools[T_CC], sizeof(cmd));

	for (;*include_dirs;include_dirs++) {
		strcat(cmd, " -I");
		strcat(cmd, *include_dirs);
	}

	strcat(cmd, " -o");
	strcat(cmd, bin_file);

	for (;*source_files;source_files++) {
		strcat(cmd, " ");
		strcat(cmd, *source_files);
	}

	return system(cmd);
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


static void write_config() {
	FILE        *fp;
	char        buf[CFG_SIZE];
	const char  *cfg_file;

	memset(config, 0, sizeof(config));
	printf_config("#ifndef UNUM_CONFIG_H");	
	printf_config("#define UNUM_CONFIG_H");	
	printf_config("");
	printf_config("/*");
	printf_config(" *  This file is auto-generated.");
	printf_config(" */");
	printf_config("");

	
	printf_config("#define UNUM_OS_WIN          %d", 
	                platform == P_WINDOWS ? 1 : 0);
	printf_config("#define UNUM_OS_MACOS        %d", 
	                platform == P_MACOS ? 1 : 0);
	printf_config("#define UNUM_OS_LINUX        %d", 
	                platform == P_LINUX ? 1 : 0);
	printf_config("");


	printf_config("#define UNUM_PATH_SEP        '%c'", path_sep);
	printf_config("#define UNUM_PATH_SEP_S      \"%c\"", path_sep);
	printf_config("");


	// - the anchor of any deployment and by putting this here it ensures
	//   that copying the deployment somewhere else will be detected by
	//   simply running `make` again.
	printf_config("#define UNUM_BASIS_DIR       \"%s\"", basis_dir);
	printf_config("");


	printf_config("#define UNUM_TOOL_CC         \"%s\"", tools[T_CC]);
	printf_config("#define UNUM_TOOL_LD         \"%s\"", tools[T_LD]);
	printf_config("");

	printf_config("#endif /* UNUM_CONFIG_H */");

	// - don't rewrite identicial content to avoid needless rebuilds
	cfg_file = to_basis("./build/include/uconfig.h");
	fp = fopen(cfg_file, "r");
	if (fp) {
		size_t rc = fread(buf, 1, CFG_SIZE, fp);
		int err   = ferror(fp);	
		fclose(fp);
		if (rc > 0 && !err && !strcmp(buf, config)) {
			return;
		}
	}

	fp = fopen(cfg_file, "w");
	if (!fp || fwrite(config, 1, strlen(config), fp) == 0) {
		abort_fail("failed to generate config '%s'", cfg_file);
	}
	fclose(fp);
}


static void printf_config(const char *fmt, ...) {
	va_list ap;
	va_start(ap, fmt);
	cfg_offset += vsnprintf(cfg_offset, sizeof(config) - (cfg_offset - config),
	                        fmt, ap);
	va_end(ap);
	*cfg_offset++ = '\n';
}
