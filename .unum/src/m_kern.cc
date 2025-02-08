/*-------------------------------------------------------------------
| vi: set noet ts=4 sw=4 fenc=utf-8
| -------------------------------------------------------------------
| Copyright (c) 2025 Francis Henry Grolemund III and RealProven, LLC.
| SPDX-License-Identifier: LicenseRef-Unum-Commercial OR GPL-3.0-only
| -------------------------------------------------------------------*/

#include <cstdio>
#include <cstring>

#include "m_kern.h"
#include "d_deploy.h"

int un::main(int argc, char **argv) {
	if (argc > 1 && !std::strcmp(argv[1], "status")) {
		int count = un::deploy_status();
		switch (count) {
		case 0:
			std::printf("no changes\n");
			break;
			
		case -1:
			std::fprintf(stderr, "unum: failed to compute status");
			return 1;
			
		default:
			std::printf("%u file%s modified\n", count, count ? "s" : "");
			break;
		}

	}
	else if (argc > 1 && !std::strcmp(argv[1], "deploy")) {
		if (argc > 2 && !std::strcmp(argv[2], "--bootstrap")) {
	    	// following a principle of trust-but-verify, the kernel validates
	    	// its own deployment created by the pre-kernel and only signs off
	    	// if it arrives at a similar result.
	    	std::printf("unum:  unum is bootstrapped.\n");
		}

	} else {
		std::printf("unum: TODO\n");
	}
	return 0;
}
