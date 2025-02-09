/*-------------------------------------------------------------------
| vi: set noet ts=4 sw=4 fenc=utf-8
| -------------------------------------------------------------------
| Copyright (c) 2025 Francis Henry Grolemund III and RealProven, LLC.
| SPDX-License-Identifier: LicenseRef-Unum-Commercial OR GPL-3.0-only
| -------------------------------------------------------------------*/


#include <cstdarg>
#include <cstdio>

#include "except.h"


un::exception::exception(const char *fmt, ...) {
	char         *buf = NULL;
	int          len  = 0;
	int          frc  = 0;
	std::va_list ap;
	
	va_start(ap, fmt);
	
	do {
		len = len + 256;
		
		if (buf) { delete [] buf; }
		buf = new char[len];
		
		frc = std::vsnprintf(buf, len, fmt, ap);
	
	} while(frc >= len);

	va_end(ap);
	message = buf;
}
	
	
un::exception::~exception() {
	if (message) {
		delete [] message;
	}
}
