/*-------------------------------------------------------------------
| vi: set noet ts=4 sw=4 fenc=utf-8
| -------------------------------------------------------------------
| Copyright (c) 2025 Francis Henry Grolemund III and RealProven, LLC.
| SPDX-License-Identifier: LicenseRef-Unum-Commercial OR GPL-3.0-only
| -------------------------------------------------------------------*/

#ifdef UNUM_BOOTSTRAP

/*
 *  Pre-kernel
 */

#include <climits>
#include <cstdlib>
#include <cstring>

#include "u_common.h"
#include "./deploy/d_deploy.h"

int main(int argc, char **argv) {
	char buf[PATH_MAX * 2];
	
	if (argc > 1 && !std::strcmp(argv[1], "deploy")) {
		if (!un::deploy(buf, sizeof(buf))) {
			std::fprintf(stderr, "unum: %s\n", buf);
			return 1;
		}
		std::strcpy(buf, UNUM_RUNTIME_BIN);
		std::strcat(buf, " deploy --bootstrap");
		if (std::system(buf) != 0) {
			std::fprintf(stderr, "unum: failed to execute bootstrapped kernel");
			return 1;
		}

	} else if (argc > 1 && (!std::strcmp(argv[1], "--version") ||
	                        !std::strcmp(argv[1], "-v"))) {
		std::printf("unum version %s\n", UNUM_VERSION_S);
		
	} else if (argc < 2 || (argc > 1 && (!std::strcmp(argv[1], "--help") ||
	                                     !std::strcmp(argv[1], "-h")))) {
		std::printf("usage: unum [-v | --version] [-h | --help] deploy\n");
		std::printf("\nNOTE: This binary is built for pre-kernel "
		            "deployment.\n");
		
	} else {
		std::fprintf(stderr, "unum: '%s' is not an unum command\n", argv[1]);
		return 1;
	
	}
	
	return 0;
}

#else

/*
 *  Kernel
 */

#include "m_kern.h"
int main(int argc, char **argv) {
	return un::main(argc, argv);
}

#endif /* UNUM_BOOTSTRAP */
