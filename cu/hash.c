/* Part of the culibs project, <http://www.eideticdew.org/culibs/>.
 * Copyright (C) 2007  Petter Urkedal <urkedal@nbi.dk>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <cu/hash.h>

#if CU_WORD_WIDTH == 32

/* cu_wordarr_hash_bj, 32 bit version
 * ----------------------------------
 *
 * A rewrite of Bob Jenkings' 32 bit mixing and hash functions from
 * from http://burtleburtle.net/bob/c/lookup3.c.  It's mostly the same as
 * the original, except for the initial seeding.  */

#define CU_HASH_BJMIX32_STEP(a, b, c)					\
    do {								\
	a -= c;  a ^= cu_hash_rot(c,  4);  c += b;			\
	b -= a;  b ^= cu_hash_rot(a,  6);  a += c;			\
	c -= b;  c ^= cu_hash_rot(b,  8);  b += a;			\
	a -= c;  a ^= cu_hash_rot(c, 16);  c += b;			\
	b -= a;  b ^= cu_hash_rot(a, 19);  a += c;			\
	c -= b;  c ^= cu_hash_rot(b,  4);  b += a;			\
    } while (0)

#define CU_HASH_BJMIX32_FINAL(a, b, c)					\
    do {								\
	c ^= b;  c -= cu_hash_rot(b, 14);				\
	a ^= c;  a -= cu_hash_rot(c, 11);				\
	b ^= a;  b -= cu_hash_rot(a, 25);				\
	c ^= b;  c -= cu_hash_rot(b, 16);				\
	a ^= c;  a -= cu_hash_rot(c,  4);				\
	b ^= a;  b -= cu_hash_rot(a, 14);				\
	c ^= b;  c -= cu_hash_rot(b, 24);				\
    } while (0)

cu_hash_t
cu_wordarr_hash_bj(size_t count, cu_word_t const *arr, cu_hash_t init)
{
    uint32_t a, b, c;
    a = 0xbe5eeded;
    b = count;
    c = init;
    /* The orginigal assigned
     * a = b = c = INT32_C(0xdeadbeef) + ((uint32_t)count << 2) + init; */
    while (count > 3) {
	a += arr[0];
	b += arr[1];
	c += arr[2];
	CU_HASH_BJMIX32_STEP(a, b, c);
	count -= 3;
	arr += 3;
    }
    switch (count) { 
	case 3: c += arr[2];
	case 2: b += arr[1];
	case 1: a += arr[0];
		CU_HASH_BJMIX32_FINAL(a, b, c);
	case 0: break;
    }
    return c;
}

cu_hash_t
cu_1word_hash_bj(cu_word_t d0, cu_hash_t init)
{
    uint32_t a, b, c;
    a = 0xbe5eeded + d0;
    b = 1;
    c = init;
    CU_HASH_BJMIX32_FINAL(a, b, c);
    return c;
}

cu_hash_t
cu_2word_hash_bj(cu_word_t d0, cu_word_t d1, cu_hash_t init)
{
    uint32_t a, b, c;
    a = 0xbe5eeded + d0;
    b = 2 + d1;
    c = init;
    CU_HASH_BJMIX32_FINAL(a, b, c);
    return c;
}

cu_hash_t
cu_wordarr_hash_noinit_bj(size_t count, cu_word_t const *arr)
{
    uint32_t a, b, c;
    a = 0xbe5eeded;
    b = count;
    c = 0;
    while (count > 3) {
	a += arr[0];
	b += arr[1];
	c += arr[2];
	CU_HASH_BJMIX32_STEP(a, b, c);
	count -= 3;
	arr += 3;
    }
    switch (count) {
	case 3: c += arr[2];
	case 2: b += arr[1];
	case 1: a += arr[0];
		CU_HASH_BJMIX32_FINAL(a, b, c);
	case 0: break;
    }
    return c;
}

cu_hash_t
cu_1word_hash_noinit_bj(cu_word_t d0)
{
    uint32_t a, b, c;
    a = 0xbe5eeded + d0;
    b = 1;
    c = 0;
    CU_HASH_BJMIX32_FINAL(a, b, c);
    return c;
}

cu_hash_t
cu_2word_hash_noinit_bj(cu_word_t d0, cu_word_t d1)
{
    uint32_t a, b, c;
    a = 0xbe5eeded + d0;
    b = 2 + d1;
    c = 0;
    CU_HASH_BJMIX32_FINAL(a, b, c);
    return c;
}

#elif CU_WORD_WIDTH == 64


/* cu_wordarr_hash_bj, 64 bit version
 * ----------------------------------
 *
 * A rewrite of Bob Jenkings' 64 bit mixing and hash functions from
 * from http://burtleburtle.net/bob/c/lookup8.c.  This computes the same
 * function as the original. */

#define CU_HASH_BJMIX64(a, b, c)					\
    do {								\
	a -= b + c;  a ^= (c >> 43);					\
	b -= c + a;  b ^= (a <<  9);					\
	c -= a + b;  c ^= (b >>  8);					\
	a -= b + c;  a ^= (c >> 38);					\
	b -= c + a;  b ^= (a << 23);					\
	c -= a + b;  c ^= (b >>  5);					\
	a -= b + c;  a ^= (c >> 35);					\
	b -= c + a;  b ^= (a << 49);					\
	c -= a + b;  c ^= (b >> 11);					\
	a -= b + c;  a ^= (c >> 12);					\
	b -= c + a;  b ^= (a << 18);					\
	c -= a + b;  c ^= (b >> 22);					\
    } while (0)

cu_hash_t
cu_wordarr_hash_bj(size_t count, cu_word_t const *arr, cu_hash_t init)
{
    uint64_t a, b, c, n;

    n = count;
    a = b = init;
    c = INT64_C(0x9e3779b97f4a7c13);
    while (n >= 3) {
	a += arr[0];
	b += arr[1];
	c += arr[2];
	CU_HASH_BJMIX64(a, b, c);
	arr += 3;
	n -= 3;
    }
    c += count << 3;
    switch (n) {
	case 2: b += arr[1];
	case 1: a += arr[0];
    }
    CU_HASH_BJMIX64(a, b, c);
    return c;
}

cu_hash_t
cu_1word_hash_bj(cu_word_t d0, cu_hash_t init)
{
    a = b = init;
    c = INT64_C(0x9e3779b97f4a7c1b); /* Added count << 3 where count = 1 */
    a += d0;
    CU_HASH_BJMIX64(a, b, c);
    return c;
}

cu_hash_t
cu_2word_hash_bj(cu_word_t d0, cu_word_t d1, cu_hash_t init)
{
    a = b = init;
    c = INT64_C(0x9e3779b97f4a7c23); /* Added count << 3 where count = 2 */
    b += d1;
    a += d0;
    CU_HASH_BJMIX64(a, b, c);
    return c;
}

cu_hash_t
cu_wordarr_hash_noinit_bj(size_t count, cu_word_t const *arr)
{
    uint64_t a, b, c, n;

    n = --count;
    a = b = *arr++;
    c = INT64_C(0x9e3779b97f4a7c13);
    while (n >= 3) {
	a += arr[0];
	b += arr[1];
	c += arr[2];
	CU_HASH_BJMIX64(a, b, c);
	arr += 3;
	n -= 3;
    }
    c += count << 3;
    switch (n) {
	case 2: b += arr[1];
	case 1: a += arr[0];
    }
    CU_HASH_BJMIX64(a, b, c);
    return c;
}

cu_hash_t
cu_1word_hash_noinit_bj(cu_word_t d0)
{
    c = INT64_C(0x9e3779b97f4a7c1b); /* Added count << 3 where count = 1 */
    b = 0;
    a = d0;
    CU_HASH_BJMIX64(a, b, c);
    return c;
}

cu_hash_t
cu_2word_hash_noinit_bj(cu_word_t d0, cu_word_t d1)
{
    c = INT64_C(0x9e3779b97f4a7c23); /* Added count << 3 where count = 2 */
    b = d1;
    a = d0;
    CU_HASH_BJMIX64(a, b, c);
    return c;
}

#else

#error Unexpected value of CU_WORD_WIDTH.

#endif
