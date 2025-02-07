/*-------------------------------------------------------------------
| vi: set noet ts=4 sw=4 fenc=utf-8
| -------------------------------------------------------------------
| Copyright (c) 2025 Francis Henry Grolemund III and RealProven, LLC.
| SPDX-License-Identifier: LicenseRef-Unum-Commercial OR GPL-3.0-only
| -------------------------------------------------------------------*/

#include <cstdio>
#include <cstring>

#include "m_kern.h"

int un::main(int argc, char **argv) {
	if (argc > 2 && !std::strcmp(argv[1], "deploy") &&
	    !std::strcmp(argv[2], "--bootstrap")) {
	    // following a principle of trust-but-verify, the kernel validates its
	    // own deployment created by the pre-kernel and only signs off if it
	    // arrives at a similar result.
	    std::printf("unum:  unum is bootstrapped.\n");
	} else {
		std::printf("unum: TODO\n");
	}
	return 0;
}
