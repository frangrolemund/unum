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
 *  platform-independent `unum` toolchain and have the latter take over further
 *  build processing.  
 *
 *  This program figures out where it is running, encodes fundamental 
 *  configuration and builds the first `unum` kernel.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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
	WINDOWS = 0,
	MACOS,
	LINUX,
	UNKNOWN	
} platform_t;

// - forward declarations
void abortFail(const char *msg);
void detectPathSep();
const char *parseOption(const char *optName, const char *from);
void parseCmdLine(int argc, char *argv[]);

// - state
char 			pathSep;
platform_t 		platformType	= UNKNOWN;
const char *	tools[T_COUNT]  = { NULL, NULL, NULL, NULL, NULL };

/*
 *  The start of it all.
 */
int main(int argc, char *argv[]) {
	detectPathSep();
	parseCmdLine(argc, argv);

	printf("inside uboot...\n");

	return 0;
}


void abortFail(const char *msg) {
	printf("uboot error: %s\n", msg);
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
