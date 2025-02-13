/*-------------------------------------------------------------------
| vi: set noet ts=4 sw=4 fenc=utf-8
| -------------------------------------------------------------------
| Copyright (c) 2025 Francis Henry Grolemund III and RealProven, LLC.
| SPDX-License-Identifier: LicenseRef-Unum-Commercial OR GPL-3.0-only
| -------------------------------------------------------------------*/

#define UNUM_UNIT_TEST 1
#include "t_test.h"
#include "ut_d_manifest.h"
using namespace un;


static test_case *next_test();


int main(int argc, char **argv) {
	std::printf("ut_all: executing unit tests\n");
	
	while (auto tptr = next_test()) {
		tptr->tprintf("--- START");
		tptr->test();
		tptr->tprintf("--- END (%d assertion%s)",
		              tptr->assert_count(),
		              tptr->assert_count() == 1 ? "" : "s");
	}
	
	std::printf("ut_all: all tests completed successfully\n");
	
	return 0;
}


test_case *next_test() {
	static test_case *item = nullptr;
	static int       count = 0;
	
	if (item) {
		delete item;
		item = nullptr;
	}
	
	switch (count++) {
	case 0:
		item = new ut_d_manifest();
		break;
		
	default:
		break;
	}

	return item;
}
