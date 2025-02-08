/*-------------------------------------------------------------------
| vi: set noet ts=4 sw=4 fenc=utf-8
| -------------------------------------------------------------------
| Copyright (c) 2025 Francis Henry Grolemund III and RealProven, LLC.
| SPDX-License-Identifier: LicenseRef-Unum-Commercial OR GPL-3.0-only
| -------------------------------------------------------------------*/

#include <cstdio>
#include <cstring>

#include "u_common.h"
#include "m_kern.h"
#include "./deploy/d_deploy.h"

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
			std::printf("%u file%s modified\n", count, (count > 1) ? "s" : "");
			break;
		}

	} else if (argc > 1 && !std::strcmp(argv[1], "deploy")) {
		char buf[256];
		if (!un::deploy(buf, sizeof(buf))) {
			std::printf("unum: failed to deploy kernel");
			return 1;
		}
		
		if (argc > 2 && !std::strcmp(argv[2], "--bootstrap")) {
			// - it would be interesting to avoid a rebuild in favor of
			//   checking the output of the bootstrapping process.
	    	std::printf("unum: unum is bootstrapped\n");
		}

	} else if (argc > 1 && (!std::strcmp(argv[1], "--version") ||
						    !std::strcmp(argv[1], "-v"))) {
		std::printf("unum version %s\n", UNUM_VERSION_S);
		
	} else if (argc > 1 && (!std::strcmp(argv[1], "--help") ||
	                        !std::strcmp(argv[1], "-h"))) {
		std::printf("usage: unum [-v | --version] [-h | --help] <command>\n");
		std::printf("\ncommands:\n");
		std::printf("   status    Show the unum deployment status\n");
		std::printf("   deploy    Rebuild and deploy the service\n");
	
	} else if (argc > 1) {
		std::printf("unum: '%s' is not an unum command.  See 'unum --help'\n",
		            argv[1]);
		return 1;
	}

	return 0;
}
