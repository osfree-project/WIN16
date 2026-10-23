/*
 * 				Shell Library Functions
 *
 * Copyright 1993, 1994, 1995 Alexandre Julliard
 * Copyright 1997 Willows Software, Inc. 
 * Copyright 1998 Marcus Meissner
 * Copyright 2000 Juergen Schmied
 * Copyright 2002 Eric Pouech
 * Copyright 2023 Yuri Prokushev
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301, USA
 */

/*! @file hashtest.c
    @brief Verify the REG.DAT hash algorithm against a known vector.

    Windows 3.1 uses `hashindex = sum(toupper(c)) % hashsize` for the
    REG.DAT hash table (see note.txt). A real REG.DAT confirms this:
    the string ".classes" ends up in bucket 0x11 (17) with hashsize 37.
*/

#include <stdio.h>
#include <ctype.h>

/*! @brief Compute the Windows 3.1 REG.DAT hash.

    Sums the toupper() code of each character and reduces modulo
    @p buckets.

    @param[in] s       Null-terminated string to hash.
    @param[in] buckets Number of hash buckets.

    @return The bucket index in [0, buckets).
*/
static unsigned int hash_sum_toupper(const char *s, unsigned int buckets)
{
    unsigned int h = 0;
    while (*s) {
        h += (unsigned char)toupper((unsigned char)*s);
        s++;
    }
    return h % buckets;
}

/*! @brief Compute the legacy XOR-based hash.

    XORs `toupper(c) + i` for each character position i, then reduces
    modulo @p buckets. This is the algorithm currently used by
    atomex.c; kept here only for comparison.

    @param[in] s       Null-terminated string to hash.
    @param[in] buckets Number of hash buckets.

    @return The bucket index in [0, buckets).
*/
static unsigned int hash_xor(const char *s, unsigned int buckets)
{
    unsigned int h = 0;
    unsigned int i = 0;
    while (*s) {
        h ^= (unsigned char)toupper((unsigned char)*s) + i;
        s++;
        i++;
    }
    return h % buckets;
}

/*! @brief Print both hashes for sample strings and self-check.

    Computes the sum(toupper) and XOR hashes for several sample strings
    and prints them side by side. Then verifies that ".classes" maps to
    bucket 0x11 (17) with 37 buckets, as observed in a real REG.DAT.

    @return 0 on success, non-zero if the self-check fails.

    @retval 0 The self-check passed.
    @retval 1 The self-check failed.
*/
int main(void)
{
    const char *test_strings[] = {
        ".classes",
        "Software",
        "Open",
        "shell",
        "command",
        NULL
    };
    unsigned int buckets = 0x25; /* 37, confirmed from a real REG.DAT */
    int i;

    printf("buckets = %u (0x%X)\n\n", buckets, buckets);
    printf("%-20s  sum(toupper)  xor\n", "String");
    printf("--------------------  ------------  ----\n");

    for (i = 0; test_strings[i]; i++) {
        unsigned int sum_hash = hash_sum_toupper(test_strings[i], buckets);
        unsigned int xor_hash = hash_xor(test_strings[i], buckets);
        printf("%-20s  %2u (0x%02X)   %2u (0x%02X)\n",
               test_strings[i],
               sum_hash, sum_hash,
               xor_hash, xor_hash);
    }

    if (hash_sum_toupper(".classes", 0x25) == 0x11) {
        printf("\nPASS: \".classes\" -> 0x11 as expected.\n");
        return 0;
    } else {
        printf("\nFAIL: \".classes\" hash mismatch!\n");
        return 1;
    }
}
