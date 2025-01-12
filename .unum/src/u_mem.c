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

#include "u_common.h"

typedef struct _uu_mem {
	short           marker;
	size_t          size;
	uu_cstring_t    file;
	int             line;
	struct _uu_mem  *prev;
	struct _uu_mem  *next;
	unsigned char   buf[];
} uu_mem_t;


static uu_mem_t *_mem_item( void *ptr );
static void mem_link( uu_mem_t *item );
static void mem_unlink( uu_mem_t *item );


static unsigned total_bytes  = 0;
static unsigned total_allocs = 0;
static uu_mem_t *memls       = NULL;


#define UU_MEM_MARK    ((short) 0x756e)
#define UU_MEM_TARE    ((short) 0x7574)
#define UU_MEM_DEALLOC ((short) -1)


void *_UU_memc_malloc( size_t size, const char *file, int line ) {
	uu_mem_t *item = NULL;

	assert(size > 0);
	
	item = malloc(sizeof(uu_mem_t) + size);
	if (!item) {
		return NULL;
	}
	
	item->marker = UU_MEM_MARK;
	item->size   = size;
	item->file   = file;
	item->line   = line;
	
	total_bytes  += size;
	total_allocs++;

	mem_link(item);

	return item->buf;
}


static void mem_link( uu_mem_t *item ) {
	item->prev = memls;
	item->next = NULL;
	
	if (memls) {
		memls->next = item;
	}
	memls = item;
}


static uu_mem_t *_mem_item( void *ptr ) {
	uu_mem_t *ret = NULL;
	
	assert(ptr);
	
	ret = (uu_mem_t *) (((char *) ptr) - sizeof(uu_mem_t));
	assert(ret->marker == UU_MEM_MARK || ret->marker == UU_MEM_TARE);

	return ret;
}


void _UU_memc_free( void *ptr ) {
	if (!ptr) {
		return;
	}

	uu_mem_t *item = _mem_item(ptr);

	if (item->marker == UU_MEM_TARE) {
		return;
	}
	
	total_bytes -= item->size;
	total_allocs--;
	
	item->marker = UU_MEM_DEALLOC;
	mem_unlink(item);
	free(item);
}


static void mem_unlink( uu_mem_t *item ) {
	uu_mem_t **to_prev = item->next ? &(item->next->prev) : &memls;
	uu_mem_t **to_next = item->prev ? &(item->prev->next) : &memls;

	*to_prev = item->prev;
	if (item->prev || memls == item) {
		*to_next = item->next;
	}	
}


void *_UU_memc_realloc( void *ptr, size_t size, const char *file, int line ) {
	uu_mem_t *item    = ptr ? _mem_item(ptr) : NULL;
	size_t 	 old_size = ptr ? item->size : 0;
	
	if (size == 0) {
		UU_mem_free(ptr);
		return NULL;
	}
	
	item = realloc(item, sizeof(uu_mem_t) + size);
	if (!item) {
		return NULL;
	}
	
	if (!ptr) {
		item->marker = UU_MEM_MARK;
		mem_link(item);
	}
	
	item->size = size;
	item->file = file;
	item->line = line;
	
	if (item->marker == UU_MEM_MARK) {
		total_bytes -= old_size;
		total_bytes += size;
	}
                            
	return item->buf;
}


void _UU_memc_tare( void *ptr ) {
	uu_mem_t *item = _mem_item(ptr);
	
	if (item->marker == UU_MEM_MARK) {
		item->marker = UU_MEM_TARE;
		total_bytes -= item->size;
		total_allocs--;
		mem_unlink(item);
	}
}


unsigned _UU_memc_num_bytes( void ) {
	return total_bytes;
}


unsigned _UU_memc_num_allocs( void ) {
	return total_allocs;
}


unsigned _UU_memc_dump ( void ) {
	uu_mem_t *cur   = memls;
	unsigned remain = 0;
	
	printf("|------ UNUM MEMORY ALLOCATIONS ------\n");
	if (!cur) {
		assert(total_bytes == 0 && total_allocs == 0);
	}

	while (cur) {
		remain++;
		printf("| %lu bytes, %s:%d\n", cur->size, cur->file, cur->line);
		cur = cur->prev;
	}
	
	if (remain) {
		if (total_allocs > 1) {
			printf("| >> %u %s in %u allocations\n", total_bytes,
			       total_bytes != 1 ? "total bytes" : "byte", total_allocs);
		}
	} else {
		printf("| >> no allocations\n");
	}
	
	printf("|------ UNUM MEMORY ALLOCATIONS ------\n\n");
	return remain;
}


extern uu_string_t UU_mem_restrcat(uu_string_t buf, uu_string_t s,
                                   size_t *nsize) {
	size_t blen = buf ? strlen(buf) : 0;
	size_t slen = s ? strlen(s) : 0;
	
	if (nsize) {
		*nsize = blen;
	}

	if (!slen) {
		return buf;
	}
	
	buf = UU_mem_realloc(buf, blen + slen + 1);
	if (!buf) {
		return NULL;
	}
	UU_mem_tare(buf);
	strcpy(&buf[blen], s);
	
	if (nsize) {
		*nsize = (size_t) (blen + slen + 1);
	}
	
	return buf;
}
