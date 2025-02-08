/*-------------------------------------------------------------------
| vi: set noet ts=4 sw=4 fenc=utf-8
| -------------------------------------------------------------------
| Copyright (c) 2025 Francis Henry Grolemund III and RealProven, LLC.
| SPDX-License-Identifier: LicenseRef-Unum-Commercial OR GPL-3.0-only
| -------------------------------------------------------------------*/

#ifndef UNUM_COMMON_H
#define UNUM_COMMON_H

#include <cstdio>
#include "u_config.h"


#define UNUM_VERSION_MAJOR     0
#define UNUM_VERSION_MINOR     1
#define UNUM_VERSION_PATCH     0
#define _unum_as_s(m)          #m
#define _unum_version(m, n, p) _unum_as_s(m.n.p)
#define UNUM_VERSION_S         _unum_version(UNUM_VERSION_MAJOR,\
                                             UNUM_VERSION_MINOR,\
											 UNUM_VERSION_PATCH)
											 
#endif /* UNUM_COMMON_H */
