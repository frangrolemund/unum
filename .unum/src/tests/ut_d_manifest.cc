/*-------------------------------------------------------------------
| vi: set noet ts=4 sw=4 fenc=utf-8
| -------------------------------------------------------------------
| Copyright (c) 2025 Francis Henry Grolemund III and RealProven, LLC.
| SPDX-License-Identifier: LicenseRef-Unum-Commercial OR GPL-3.0-only
| -------------------------------------------------------------------*/


#define UNUM_UNIT_TEST 1
#include "ut_d_manifest.h"
using namespace un;


ccstring_t ut_d_manifest::name( void ) { return "ut_manifest"; }


void ut_d_manifest::test( void ) {
	tprintf("inside test");
	test_assert(1 == 1, "simple proof");
}
