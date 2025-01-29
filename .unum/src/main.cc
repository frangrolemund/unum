/*-------------------------------------------------------------------
| vi: set noet ts=4 sw=4 fenc=utf-8
| -------------------------------------------------------------------
| Copyright (c) 2025 Francis Henry Grolemund III and RealProven, LLC.
| SPDX-License-Identifier: LicenseRef-Unum-Commercial OR GPL-3.0-only
| -------------------------------------------------------------------*/

#include <string.h>

#include "u_common.h"
#include "./deploy/d_deploy.h"


int main(int argc, char **argv) {
	std::printf("TODO: unum version %s\n", UNUM_VERSION_S);
	if (argc > 1 && !strcmp(argv[1], "deploy")) {
		unum::deploy();
	}
	return 0;
}
