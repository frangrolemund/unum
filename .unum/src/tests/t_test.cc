/*-------------------------------------------------------------------
| vi: set noet ts=4 sw=4 fenc=utf-8
| -------------------------------------------------------------------
| Copyright (c) 2025 Francis Henry Grolemund III and RealProven, LLC.
| SPDX-License-Identifier: LicenseRef-Unum-Commercial OR GPL-3.0-only
| -------------------------------------------------------------------*/

#include <cassert>
#include <cstdarg>
#include <cstdlib>

#define UNUM_UNIT_TEST 1
#include "t_test.h"
using namespace un;


test_case::test_case( void )
	: _assert_count{0} {
}


int test_case::assert_count( void ) { return _assert_count; }


test_case::~test_case( void ) {
}


void test_case::test( void ) {
}


void test_case::tprintf(ccstring_t fmt, ...) {
	std::va_list ap;

	fprintf(stdout, "%s: ", this->name());
	va_start(ap, fmt);
	vfprintf(stdout, fmt, ap);
	fprintf(stdout, "\n");
	va_end(ap);
}


void test_case::_test_assert( bool expr_val, ccstring_t expr, ccstring_t file,
                              int line, ccstring_t msg ) {
	_assert_count++;
	if (expr_val) {
		return;
	}

	fprintf(stderr, "%s: TEST-FAILED: %s <-- (%s)\n", this->name(), msg, expr);
	fprintf(stderr, "%s: TEST-FAILED @%s:%d\n", this->name(), file, line);
	exit(1);
}
