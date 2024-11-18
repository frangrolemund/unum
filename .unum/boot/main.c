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
 *  configuration and builds the first `unum` kernel.  The objective is for
 *  it to be as minimal as possible without confusing the process.
 */

#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

// - types
typedef enum {
	T_CC = 0,
	T_CFLAGS,
	T_LD,
	T_LDFLAGS,
	T_LDLIBS,

	T_COUNT
} tool_e;

static const struct { tool_e tool; const char *oname; } tool_map[] = {
	{ T_CC,      "cc" },
	{ T_CFLAGS,  "ccflags" },
	{ T_LD,      "ld" },
	{ T_LDFLAGS, "ldflags" },
	{ T_LDLIBS,  "ldlibs" }
};

typedef enum {
	P_WINDOWS = 0,
	P_MACOS,
	P_LINUX,
	P_UNKNOWN	
} platform_e;

// - forward declarations
static void abort_fail(const char *msg);
static void detect_path_style();
static void detect_platform();
static int run_cc(const char *code);
static const char *parse_option(const char *optName, const char *from);
static void parse_cmd_line(int argc, char *argv[]);

// - state
static char        path_sep;
static platform_e  platform        = P_UNKNOWN;
static const char  *tools[T_COUNT] = { NULL, NULL, NULL, NULL, NULL };
static FILE        *uberr          = NULL;


int main(int argc, char *argv[]) {
	// - don't spam stderr with system()
	uberr = fdopen(dup(fileno(stderr)), "w");
	fclose(stderr);

	parse_cmd_line(argc, argv);
	detect_path_style();
	detect_platform();

	

	printf("inside uboot...\n");
	printf("- path separator: '%c'\n", path_sep);
	printf("- platform:  	   %d\n", platform);
	for (int i = 0; i < T_COUNT; i++) {
	printf("- tool %d:         %s\n", i, tools[i]);
	}

	fclose(uberr);
	return 0;
}


static void abort_fail(const char *msg) {
	fprintf(uberr ? uberr : stdout, "uboot error: %s\n", msg);
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
	char prefix[64];

	snprintf(prefix, sizeof(prefix), "--%s=", opt_name);
	if (strncmp(from, prefix, strlen(prefix))) {
		return NULL;	
	}

	return from + strlen(prefix);
}

static void parse_cmd_line(int argc, char *argv[]) {
	const char  *opt = NULL;
	int         i;

	while (--argc) {
		const char *item = *++argv;

		for (i = 0; i < sizeof(tool_map)/sizeof(tool_map[0]); i++) {
			if ((opt = parse_option(tool_map[i].oname, item))) {
				tools[tool_map[i].tool] = opt;
			}	
		}
	}

	if (!tools[T_CC] || !tools[T_LD]) {
		abort_fail("missing one or more required tool parameters.");
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
