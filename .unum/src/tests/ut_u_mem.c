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

#include "u_common.h"
#include "u_test.h"
#include "u_mem.h"

static int unittest_mem( int argc, char *argv[] );
static void mem_test_simple( void );
static void mem_test_list( void );

int main( int argc, char *argv[] ) {
	return UT_test(argc, argv, unittest_mem);
}


static int unittest_mem( int argc, char *argv[] ) {
	mem_test_simple();
	mem_test_list();
	return 0;
}


static void mem_test_simple( void ) {
	uu_cstring_t	tests[]       = { "apple", "pear", "grapes", "kiwi" };
	uu_cstring_t    tares[]       = { "bear", "lion", "giraffe" };
	uu_string_t     allocs[32];
	int             i, count      = 0;
	size_t          expected_size = 0;
	const int       num_tests     = sizeof(tests)/sizeof(tests[0]);
	
	UT_set_test_name("simple allocation patterns");
	UT_printf("verifying basic accounting...");
	
	for (i = 0; i < num_tests; i++) {
		allocs[count++] = UU_strdup(tests[i]);
		expected_size += strlen(tests[i]) + 1;
		
		if (i < sizeof(tares)/sizeof(tares[0])) {
			allocs[count] = UU_strdup(tares[i]);
			UU_mem_tare(allocs[count++]);
		}
	}

	UT_assert(expected_size == UU_memdbg_total_bytes(), "invalid byte count");
	UT_assert(UU_memdbg_total_allocs() == num_tests, "invalid alloc count");
	
	for (i = 0, count = 0; i < num_tests; i++) {
		UT_assert(!strcmp(allocs[count++], tests[i]), "unexpected string");
		
		if (i < sizeof(tares)/sizeof(tares[0])) {
			UT_assert(!strcmp(allocs[count++], tares[i]), "unexpected string");
		}
	}
	
	UT_assert(UU_memdbg_dump() == num_tests, "Unexpected dump.");
	
	for (i = 0; i < count; i++) {
		// - should be non-destructive on tared data
		UU_free(allocs[i]);
	}
	
	UT_assert(UU_memdbg_total_bytes() == 0, "invalid byte count");
	UT_assert(UU_memdbg_total_allocs() == 0, "invalid alloc count");
}


static void mem_test_list( void ) {
	uu_string_t one, two, three, four, five;

	UT_set_test_name("memory list verification");
	
	UT_printf("allocating items...");
	one   = UU_strdup("one");
	two   = UU_strdup("two");
	three = UU_strdup("three");
	four  = UU_strdup("four");
	five  = UU_strdup("five");
	
	UT_assert(one && two && three && four && five, "alloc failed");
	
	UT_printf("performing non-sequential deallocations...");
	UT_assert(UU_memdbg_dump() == 5, "unexpected number of allocs");
	
	UU_free(three);
	UT_assert(UU_memdbg_dump() == 4, "unexpected number of allocs");
	
	UU_free(one);
	UT_assert(UU_memdbg_dump() == 3, "unexpected number of allocs");
	
	UU_free(five);
	UT_assert(UU_memdbg_dump() == 2, "unexpected number of allocs");
	
	UU_free(two);
	UT_assert(UU_memdbg_dump() == 1, "unexpected number of allocs");
	
	UU_free(four);
	UT_assert(UU_memdbg_dump() == 0, "unexpected number of allocs");
	UT_assert(UU_memdbg_total_bytes() == 0, "unexpected number of bytes.");
	UT_assert(UU_memdbg_total_allocs() == 0, "unexpected number of allocs.");
}
