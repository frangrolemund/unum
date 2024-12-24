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
static void mem_test_realloc( void );


int main( int argc, char *argv[] ) {
	return UT_test_run(argc, argv, unittest_mem);
}


static int unittest_mem( int argc, char *argv[] ) {
	mem_test_simple();
	mem_test_list();
	mem_test_realloc();
	return 0;
}


static void mem_test_simple( void ) {
	uu_cstring_t	tests[]       = { "apple", "pear", "grapes", "kiwi" };
	uu_cstring_t    tares[]       = { "bear", "lion", "giraffe" };
	uu_string_t     allocs[32];
	int             count         = 0;
	size_t          expected_size = 0;
	const int       num_tests     = sizeof(tests)/sizeof(tests[0]);
	
	UT_test_setname("simple allocation patterns");
	UT_test_printf("checking basic");
	
	for (int i = 0; i < num_tests; i++) {
		allocs[count++] = UU_mem_strdup(tests[i]);
		expected_size += strlen(tests[i]) + 1;
		
		if (i < sizeof(tares)/sizeof(tares[0])) {
			allocs[count] = UU_mem_strdup(tares[i]);
			UU_mem_tare(allocs[count++]);
		}
	}

	UT_test_assert(expected_size == UU_memc_num_bytes(), "invalid byte count");
	UT_test_assert(UU_memc_num_allocs() == num_tests, "invalid alloc count");
	
	for (int i = 0, count = 0; i < num_tests; i++) {
		UT_test_assert_eq(allocs[count++], tests[i], "unexpected string");
		
		if (i < sizeof(tares)/sizeof(tares[0])) {
			UT_test_assert_eq(allocs[count++], tares[i], "unexpected string");
		}
	}
	
	UT_test_assert(UU_memc_dump() == num_tests, "Unexpected dump.");
	
	for (int i = 0; i < count; i++) {
		// - should be non-destructive on tared data
		UU_mem_free(allocs[i]);
	}
	
	UT_test_assert(UU_memc_num_bytes() == 0, "invalid byte count");
	UT_test_assert(UU_memc_num_allocs() == 0, "invalid alloc count");
}


static void mem_test_list( void ) {
	uu_string_t one, two, three, four, five;

	UT_test_setname("memory list checks");
	
	UT_test_printf("allocating items");
	one   = UU_mem_strdup("one");
	two   = UU_mem_strdup("two");
	three = UU_mem_strdup("three");
	four  = UU_mem_strdup("four");
	five  = UU_mem_strdup("five");
	
	UT_test_assert(one && two && three && four && five, "alloc failed");
	
	UT_test_printf("check non-sequential deallocs");
	UT_test_assert(UU_memc_dump() == 5, "unexpected number of allocs");
	
	UU_mem_free(three);
	UT_test_assert(UU_memc_dump() == 4, "unexpected number of allocs");
	
	UU_mem_free(one);
	UT_test_assert(UU_memc_dump() == 3, "unexpected number of allocs");
	
	UU_mem_free(five);
	UT_test_assert(UU_memc_dump() == 2, "unexpected number of allocs");
	
	UU_mem_free(two);
	UT_test_assert(UU_memc_dump() == 1, "unexpected number of allocs");
	
	UU_mem_free(four);
	UT_test_assert(UU_memc_dump() == 0, "unexpected number of allocs");
	UT_test_assert(UU_memc_num_bytes() == 0, "unexpected number of bytes.");
	UT_test_assert(UU_memc_num_allocs() == 0, "unexpected number of allocs.");
}

#define mem_assert_char(p, c, n)     \
					for (int i = 0; i < (n); i++) {\
						UT_test_assert(p[i] == (c),\
							"failed to find prior value");\
		            }


static void mem_test_realloc( void ) {
	uu_string_t s0, s1, first_buf, buf, b2;
	size_t last_count, count, other;
	char c                             = 'a';
	
	UT_test_setname("realloc checks");
	
	last_count      = 256;
	s0              = UU_mem_strdup("abc");             // ...not first
	first_buf = buf = UU_mem_alloc(last_count);
	s1              = UU_mem_strdup("xyz");             // ...not last
	b2              = UU_mem_realloc(NULL, last_count);	// ...alternate alloc
	UT_test_assert(s0 && buf && s1 && b2, "out of memory");
	UU_mem_set(buf, c, last_count);
	UU_mem_set(b2, c, last_count);
	
	UT_test_printf("expanding buffer");
	other = strlen(s0) + 1 + strlen(s1) + 1 + last_count;
	for (count = 1024; count < 0x8FFFF; count += 0x0FFF) {
		buf = UU_mem_realloc(buf, count);
		UT_test_assert(buf, "out of memory");
		UT_test_assert(UU_memc_num_allocs() == 3, "unexpected allocs");
		UT_test_assert(UU_memc_num_bytes() == count + other,
		               "unexpected total");
		
		mem_assert_char(buf, c, last_count);
		
		UU_mem_set(buf, ++c, count);
		last_count = count;
	}
	
	UT_test_assert(buf != first_buf, "unexpected unmoved pointer")
	
	UT_test_printf("compressing buffer");
	count >>= 1;
	buf = UU_mem_realloc(buf, count);
	UT_test_assert(buf, "out of memory");
	mem_assert_char(buf, c, last_count);
	UU_mem_set(buf, ++c, count);
	UT_test_assert(UU_memc_num_bytes() == count + other, "unexpected total");
	
	UT_test_printf("taring buffer");
	UU_mem_tare(buf);
	UT_test_assert(UU_memc_num_allocs() == 2, "unexpected allocs");
	UT_test_assert(UU_memc_num_bytes() == other, "unexpected total");
	last_count = count;
	count      += 0xFEDC;		// - arbitrary, not power of 2
	buf        = UU_mem_realloc(buf, count);
	UT_test_assert(buf, "out of memory");
	mem_assert_char(buf, c, last_count);
	UU_mem_set(buf, ++c, count);
	
	UU_mem_realloc(buf, 0);
	UT_test_assert(UU_memc_num_allocs() == 2, "unexpected allocs");
	UT_test_assert(UU_memc_num_bytes() == other, "unexpected total");
	
	UU_mem_realloc(s0, 0);
	UU_mem_realloc(s1, 0);
	UU_mem_realloc(b2, 0);
}
