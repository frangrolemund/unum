/*-------------------------------------------------------------------
| vi: set noet ts=4 sw=4 fenc=utf-8
| -------------------------------------------------------------------
| Copyright (c) 2025 Francis Henry Grolemund III and RealProven, LLC.
| SPDX-License-Identifier: LicenseRef-Unum-Commercial OR GPL-3.0-only
| -------------------------------------------------------------------*/

#define UNUM_UNIT_TEST 1
#include "t_test.h"


// -- UNUM NAMESPACE
namespace un {


class ut_d_manifest : public un::test_case {
	ccstring_t name( void ) override;
	void test( void ) override;

};


}

