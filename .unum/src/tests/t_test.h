/*-------------------------------------------------------------------
| vi: set noet ts=4 sw=4 fenc=utf-8
| -------------------------------------------------------------------
| Copyright (c) 2025 Francis Henry Grolemund III and RealProven, LLC.
| SPDX-License-Identifier: LicenseRef-Unum-Commercial OR GPL-3.0-only
| -------------------------------------------------------------------*/

#ifndef UNUM_TEST_H
#define UNUM_TEST_H


#include "u_common.h"


#ifndef UNUM_UNIT_TEST
#error "Unit testing only."
#endif


#define test_assert( t, m )     _test_assert(!!(t), #t, __FILE__, __LINE__, (m))


// -- UNUM NAMESPACE
namespace un {


class test_case {
	public:
	
	test_case( void );
	virtual ccstring_t name( void ) = 0;
	int assert_count( void );
	virtual void test( void );
	virtual ~test_case( void );
	void tprintf( ccstring_t fmt, ... );
	
	protected:
	
	void _test_assert( bool expr_val, ccstring_t expr, ccstring_t file,
	                   int line, ccstring_t msg );
	                   
	private:
	int _assert_count;
	
};


}


#endif
