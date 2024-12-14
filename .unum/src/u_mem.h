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

#ifndef UNUM_MEM_H
#define UNUM_MEM_H

#include <stdlib.h>
#include <string.h>

#ifdef UNUM_UNIT_TEST
#define UNUM_MEM_CHECKING
#endif

#ifdef UNUM_MEM_CHECKING
	extern void     *_UU_memcheck_malloc( size_t size, const char *file,
	                                      int line );
	extern void     _UU__memcheck_free( void *ptr );
	extern void     *_UU_memcheck_realloc( void *ptr, size_t size,
                                           const char *file, int line );
	extern void     _UU_memcheck_tare( void *ptr );
	extern unsigned _UU_memcheck_total_bytes( void );
	extern void     _UU_memcheck_dump ( void );

	#define UU_malloc(n)              _UU_memcheck_malloc((n), __FILE__, \
	                                                      __LINE__)
	#define UU_free(p)                _UU__memcheck_free(p)
	#define UU_realloc(p, n)          _UU_memcheck_realloc((p), (n), __FILE__, \
                                                           __LINE__)
	#define UU_tare(p)                _UU_memcheck_tare((p))
	#define UU_memcheck_total_bytes() _UU_memcheck_total_bytes()
	#define UU_memcheck_dump()        _UU_memcheck_dump()
  
#else

	#define UU_malloc(n)              malloc(n)
	#define UU_free(p)                free(p)
	#define UU_realloc(p, n)          realloc((p), (n))
  
	#define UU_tare(p)                assert(p != NULL)
	#define UU_memcheck_total_bytes() ((unsigned) 0)
	#define UU_memcheck_dump()        assert(0)
  
#endif /* UNUM_MEM_CHECKING */


#define UU_memset(p, v, n)   memset((p), (v), (n))
#define UU_memcpy(d, s, n)   memcpy((d), (s), (n))

#endif /* UNUM_MEM_H */
