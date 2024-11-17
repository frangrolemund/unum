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
} tool_t;

typedef enum {
	P_WINDOWS = 0,
	P_MACOS,
	P_LINUX,
	P_UNKNOWN	
} platform_t;

// - forward declarations
void abortFail(const char *msg);
void detectPathSep();
void detectPlatform();
int execCC(const char *code);
const char *parseOption(const char *optName, const char *from);
void parseCmdLine(int argc, char *argv[]);

// - state
char 			pathSep;
platform_t 		platform 		= P_UNKNOWN;
const char *	tools[T_COUNT]  = { NULL, NULL, NULL, NULL, NULL };
FILE *			altErr			= NULL;


int main(int argc, char *argv[]) {
	// - don't spam stderr with system()
	altErr = fdopen(dup(fileno(stderr)), "w");
	fclose(stderr);

	parseCmdLine(argc, argv);
	detectPathSep();
	detectPlatform();

	

	printf("inside uboot...\n");

	return 0;
}


void abortFail(const char *msg) {
	fprintf(altErr ? altErr : stdout, "uboot error: %s\n", msg);
	exit(1);
}


void detectPathSep() {
	const char *path = getenv("PATH");
	char c;

	while (path && (c = *path++)) {
		if (c == '/' || c == '\\') {
			pathSep = c;
			return;
		}
	}

	abortFail("missing PATH environment");
}

const char *parseOption(const char *optName, const char *from) {
	char optPrefix[64];
	snprintf(optPrefix, sizeof(optPrefix), "--%s=", optName);
	if (strncmp(from, optPrefix, strlen(optPrefix))) {
		return NULL;	
	}
	return from + strlen(optPrefix);
}

void parseCmdLine(int argc, char *argv[]) {
	const char *opt = NULL;
	int i;
	while (--argc) {
		const char *item = *++argv;
		if ((opt = parseOption("cc", item))) {
			tools[T_CC] = opt;
		} else if ((opt = parseOption("ccflags", item))) {
			tools[T_CFLAGS] = opt;
		} else if ((opt = parseOption("ld", item))) {
			tools[T_LD] = opt;
		} else if ((opt = parseOption("ldflags", item))) {
			tools[T_LDFLAGS] = opt;
		} else if ((opt = parseOption("ldlibs", item))) {
			tools[T_LDLIBS] = opt;
		}
	}

	for (i = 0; i < T_COUNT; i++) {
		if (!tools[i]) {
			abortFail("missing one or more required tool parameters.");
		}
	}
}

int execCC(const char *source) {
	const char *tmpEnv[] 	= { "TMPDIR", "TMP", "TEMP", "TEMPDIR", NULL };
	int i, rc;
	char srcName[PATH_MAX]	= "\0";
	char binName[PATH_MAX];
	char ccCmd[2048];
	FILE *srcFile;

	// ...avoids the scary warning from macos for using tmpnam
	for (i = 0; tmpEnv[i]; i++) {
		const char *eVal;
		if ((eVal = getenv(tmpEnv[i]))) {
			snprintf(srcName, PATH_MAX, "%s%cunum-boot.c", eVal, pathSep);
			break;
		}
	}

	if (!srcName[0]) {
		abortFail("failed to find temp directory.");
	}

	snprintf(binName, PATH_MAX, "%s.out", srcName);

	srcFile = fopen(srcName, "w");
	if (!srcFile || !fwrite(source, strlen(source), 1, srcFile)) {
		abortFail("failed to create temp file.");
	}
	fclose(srcFile);

	snprintf(ccCmd, sizeof(ccCmd), "%s -o %s %s", 
				tools[T_CC], binName, srcName);

	rc = system(ccCmd);	
	unlink(srcName);
	unlink(binName);

	return rc;
}

void detectPlatform() {
	if (execCC(	"#include <Carbon/Carbon.h>\n"	
				"#include <stdio.h>\n\n"
				"int main(int argc, char **argv) {\n"
				"  printf(\"hello unum\");\n"
				"}\n") == 0) {
		platform = P_MACOS;
		return;
	}

	abortFail("unsupported platform type");
}
