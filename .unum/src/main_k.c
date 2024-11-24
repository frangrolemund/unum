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
#include "uconfig.h"

/*
 *  Kernel:  The binary build of `unum` that includes the entire systemic
 *           programming toolchain and the native implementation of the latest
 *           deployed main system.  All facilities of runtime, analytics,
 *           tooling, networking, etc, are built into a single executable that
 *           maintains the deployment and its dependencies on physical
 *           system resources.
 */

#ifdef UNUM_BOOTSTRAP
#error "Kernel deployment only."
#endif

int UM_main(int argc, char *argv[]) {
	printf("DEBUG: in kernel, hello unum --> %s.\n", UNUM_TOOL_CC);
	return 0;
}
