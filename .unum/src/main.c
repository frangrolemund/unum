/*---------------------------------------------------------------
| vi: set noet ts=4 sw=4 fenc=utf-8
| ---------------------------------------------------------------
| Copyright 2024 Francis Henry Grolemund III
|   
| Permission to use, copy, modify, and/or distribute this software for
| any purpose with or without fee is hereby granted, provided that the
| above copyright notice and this permission notice appear in all copies.
|   
| THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL
| WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED
| WARRANTIES OF MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE
| AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL
| DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR
| PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER
| TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
| PERFORMANCE OF THIS SOFTWARE. 
| ---------------------------------------------------------------*/

#include <stdio.h>

#ifdef UNUM_BOOTSTRAP

	/*
	 *  The pre-kernel of unum is run from `make` and only as much as is
	 *  necessary to deploy the full kernel from source so that the build
	 *  machinery is used for every full build of the kernel, including the
	 *  first.
 	 */
	#include "main_pre_k.c"

#else 

	/*
 	 *  The unum kernel manages the entire systemic development pipeline 
	 *  (source, build, runtime, analytics, networking, logging) for a 
     *  deployment and its integration with the basis.
 	 */
	#include "main_k.c"

#endif /* UNUM_BOOTSTRAP */
