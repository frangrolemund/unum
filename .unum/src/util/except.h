/*-------------------------------------------------------------------
| vi: set noet ts=4 sw=4 fenc=utf-8
| -------------------------------------------------------------------
| Copyright (c) 2025 Francis Henry Grolemund III and RealProven, LLC.
| SPDX-License-Identifier: LicenseRef-Unum-Commercial OR GPL-3.0-only
| -------------------------------------------------------------------*/

#ifndef UNUM_EXCEPT_H
#define UNUM_EXCEPT_H


// -- UNUM NAMESPACE
namespace un {


class exception {
	public:
	
	const char *message;


	exception(const char *fmt, ...);
	~exception();

};


}


#endif /* UNUM_EXCEPT_H */
