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

#include <stdio.h>
#include <string.h>

#include "u_common.h"
#include "deploy.h"

#ifndef UNUM_BOOTSTRAP
#error "Bootstrapping only."
#endif

static void print_usage( void );

/*
 *  Pre-kernel: The binary build of `unum` created when preparing a repo
 *              with the Makefile for a new compiler toolchain or platform.
 *              It includes the minimal amount of infrastructure to build 
 *              previously-deployed unum kernel code using the same mechanisms 
 *              used in the unum workflows.  The pre-kernel is built by the 
 *              bootstrapping control (uboot) using a brute-forced approach 
 *              without providing a lot of feedback on  errors.  The pre-kernel
 *              can then rebuild the full kernel using the official machinery of
 *              deployment, which includes more precise error reporting.
 */

int UM_main( int argc, char *argv[] ) {
	if (argc != 2) {
		print_usage();
		return 1;
	}
	
	if (!strcmp(argv[1], "deploy")) {
		printf("TODO: deploy\n");
		UD_deploy(UD_DEPLOY_CLEAN);
		printf("unum is bootstrapped\n");
		
	} else if (!strcmp(argv[1], "--version")) {
		printf("unum pre-k version %s\n", UNUM_VERSION.as_string);
		
	} else if (!strcmp(argv[1], "--help")) {
		print_usage();
		
	}
	
	return 0;
}


static void print_usage( void ) {
	printf("usage: unum deploy\n");
	printf("       unum --version\n");
	printf("       unum --help\n");
}
