/*
 * 				REG.DAT parser and validator
 *
 * Copyright 2023 Yuri Prokushev
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 */

/*! @file regparse.c
    @brief Host-side REG.DAT parser and validator.

    Reads a Windows 3.1 REG.DAT, validates its header, hash chains,
    navigation tree, text block and free list. Verifies the REG.DAT
    hash formula (sum(toupper) % hashsize) against every string in
    the file.

    Note: strings in the text block are NOT NUL-terminated. Each
    string is preceded by a 16-bit word (2*index+1) and its length
    is stored in the corresponding DATTEXT record. See note.txt.

    Usage:  regparse [file] [-v] [-q]

    Exit code: 0 on success, 1 on any parse/validation failure.
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

/* ------------------------------------------------------------------ */
/* Little-endian readers (safe on big-endian hosts)                    */
/* ------------------------------------------------------------------ */

/*! @brief Read a little-endian 32-bit word.

    @param[in] p Pointer to 4 bytes.

    @return The value as unsigned int.
*/
static unsigned int read_le32(const unsigned char *p)
{
    return (unsigned int)p[0]
         | ((unsigned int)p[1] << 8)
         | ((unsigned int)p[2] << 16)
         | ((unsigned int)p[3] << 24);
}

/*! @brief Read a little-endian 16-bit word.

    @param[in] p Pointer to 2 bytes.

    @return The value as unsigned int.
*/
static unsigned int read_le16(const unsigned char *p)
{
    return (unsigned int)p[0]
         | ((unsigned int)p[1] << 8);
}

/* ------------------------------------------------------------------ */
/* REG.DAT header field offsets                                        */
/* ------------------------------------------------------------------ */

/*! @def HDR_SIG
    @brief Header offset of the signature field ("SHCC3.10"). */
#define HDR_SIG        0

/*! @def HDR_UNK1
    @brief Header offset of unk1: hash-table offset (??); may be header size. */
#define HDR_UNK1       8

/*! @def HDR_UNK2
    @brief Header offset of unk2: index-table offset (??); always equals unk1. */
#define HDR_UNK2      12

/*! @def HDR_DWENTRIES
    @brief Header offset of the total 8-byte entry count. */
#define HDR_DWENTRIES 16

/*! @def HDR_OFFTEXT
    @brief Header offset of the text-block offset. */
#define HDR_OFFTEXT   20

/*! @def HDR_SIZETEXT
    @brief Header offset of the text-block size. */
#define HDR_SIZETEXT  24

/*! @def HDR_HASHSIZE
    @brief Header offset of the number of hash buckets. */
#define HDR_HASHSIZE  28

/*! @def HDR_FREEIDX
    @brief Header offset of the first free entry index. */
#define HDR_FREEIDX   30

/*! @def HDR_SIZE
    @brief Total header size in bytes. */
#define HDR_SIZE      32

/*! @def REC_SIZE
    @brief Size of one entry-table record in bytes. */
#define REC_SIZE       8

/* ------------------------------------------------------------------ */
/* Runtime view                                                        */
/* ------------------------------------------------------------------ */

/*! @brief Parsed view of a REG.DAT file. */
typedef struct {
    unsigned char *raw;        /*!< Raw file contents. */
    unsigned int   size;       /*!< Total file size. */

    unsigned int   unk1;       /*!< Header field at 0x08. */
    unsigned int   unk2;       /*!< Header field at 0x0C. */
    unsigned int   dwEntries;  /*!< Header field at 0x10. */
    unsigned int   offText;    /*!< Header field at 0x14. */
    unsigned int   sizeText;   /*!< Header field at 0x18. */
    unsigned int   hashsize;   /*!< Header field at 0x1C. */
    unsigned int   freeidx;    /*!< Header field at 0x1E. */

    const unsigned char *tab;  /*!< Pointer to the entry table. */
    const unsigned char *text; /*!< Pointer to the text block. */
} REGDAT;

/*! @brief Counters accumulated during validation. */
typedef struct {
    unsigned int checks;   /*!< Number of checks performed. */
    unsigned int errors;   /*!< Number of errors detected. */
} STATS;

/*! @def KNOWN_STRING
    @brief String used as a known-vector for the hash check. */
#define KNOWN_STRING      ".classes"

/*! @def KNOWN_HASH
    @brief Expected hash bucket for KNOWN_STRING. */
#define KNOWN_HASH        0x11

/*! @def KNOWN_HASHSIZE
    @brief Expected hashsize for KNOWN_STRING. */
#define KNOWN_HASHSIZE    37

/* ------------------------------------------------------------------ */
/* File I/O                                                            */
/* ------------------------------------------------------------------ */

/*! @brief Load a file into memory.

    @param[in]  path     Path to the file.
    @param[out] out_size Receives the file size in bytes.

    @return Pointer to the file contents (malloc'd), or NULL on failure.

    @retval NULL The file could not be read or memory allocation failed.
*/
static unsigned char *read_file(const char *path, unsigned int *out_size)
{
    FILE *fp;
    long   sz;
    unsigned char *buf;

    fp = fopen(path, "rb");
    if (!fp) return NULL;

    if (fseek(fp, 0, SEEK_END) != 0) { fclose(fp); return NULL; }
    sz = ftell(fp);
    if (sz < 0) { fclose(fp); return NULL; }
    if (fseek(fp, 0, SEEK_SET) != 0) { fclose(fp); return NULL; }

    buf = (unsigned char *)malloc((size_t)sz);
    if (!buf) { fclose(fp); return NULL; }

    if (fread(buf, 1, (size_t)sz, fp) != (size_t)sz) {
        free(buf);
        fclose(fp);
        return NULL;
    }
    fclose(fp);

    *out_size = (unsigned int)sz;
    return buf;
}

/* ------------------------------------------------------------------ */
/* Header parsing + layout checks                                      */
/* ------------------------------------------------------------------ */

/*! @brief Parse and validate the REG.DAT header.

    @param[in,out] r       Parsed view (fields filled in on success).
    @param[in,out] s       Stats.
    @param[in]     verbose Verbose output flag.

    @return 1 if the header is valid, 0 otherwise.

    @retval 1 The header is valid and all layout checks passed.
    @retval 0 The header is invalid or a layout check failed.
*/
static int parse_header(REGDAT *r, STATS *s, int verbose)
{
    if (r->size < HDR_SIZE) {
        printf("ERROR: file too short for REG.DAT header (%u bytes)\n", r->size);
        return 0;
    }
    if (memcmp(r->raw + HDR_SIG, "SHCC3.10", 8) != 0) {
        printf("ERROR: bad signature \"%.8s\" (expected \"SHCC3.10\")\n",
               (char *)r->raw + HDR_SIG);
        return 0;
    }

    r->unk1      = read_le32(r->raw + HDR_UNK1);
    r->unk2      = read_le32(r->raw + HDR_UNK2);
    r->dwEntries = read_le32(r->raw + HDR_DWENTRIES);
    r->offText   = read_le32(r->raw + HDR_OFFTEXT);
    r->sizeText  = read_le32(r->raw + HDR_SIZETEXT);
    r->hashsize  = read_le16(r->raw + HDR_HASHSIZE);
    r->freeidx   = read_le16(r->raw + HDR_FREEIDX);

    if (verbose) {
        printf("Header:\n");
        printf("  Signature         %.8s\n", (char *)r->raw + HDR_SIG);
        printf("  unk1              0x%08X\n", r->unk1);
        printf("  unk2              0x%08X\n", r->unk2);
        printf("  dwEntries         %u\n", r->dwEntries);
        printf("  offsetDataBlock   0x%08X\n", r->offText);
        printf("  sizeDataBlock     0x%08X (%u)\n", r->sizeText, r->sizeText);
        printf("  hashsize          %u\n", r->hashsize);
        printf("  freeidx           %u\n", r->freeidx);
        printf("\n");
    }

    if (r->dwEntries == 0) { printf("ERROR: dwEntries == 0\n"); s->errors++; return 0; }
    if (r->hashsize == 0)  { printf("ERROR: hashsize == 0\n");  s->errors++; return 0; }
    if (r->dwEntries < r->hashsize + 1) {
        printf("ERROR: dwEntries (%u) < hashsize+1 (%u)\n",
               r->dwEntries, r->hashsize + 1);
        s->errors++;
        return 0;
    }
    if (r->unk1 < HDR_SIZE || r->unk1 > r->size) {
        printf("ERROR: unk1 (%u) outside file size (%u)\n", r->unk1, r->size);
        s->errors++;
        return 0;
    }
    if (r->dwEntries > (r->size - r->unk1) / REC_SIZE) {
        printf("ERROR: entry table extends past end of file\n");
        s->errors++;
        return 0;
    }
    if (r->offText > r->size || r->sizeText > r->size - r->offText) {
        printf("ERROR: text block extends past end of file\n");
        s->errors++;
        return 0;
    }

    r->tab  = r->raw + r->unk1;
    r->text = r->raw + r->offText;
    s->checks += 6;
    return 1;
}

/* ------------------------------------------------------------------ */
/* DATTEXT accessors                                                   */
/* ------------------------------------------------------------------ */

/*! @brief Read DATTEXT.Index (next-in-chain) for record @p i.

    @param[in] r Parsed REG.DAT.
    @param[in] i Record index.

    @return Index of the next record in the hash chain.
*/
static unsigned int dt_index(const REGDAT *r, unsigned int i)
{
    return read_le16(r->tab + i * REC_SIZE + 0);
}

/*! @brief Read DATTEXT.Length for record @p i.

    @param[in] r Parsed REG.DAT.
    @param[in] i Record index.

    @return String length in bytes (no NUL terminator in text block).
*/
static unsigned int dt_length(const REGDAT *r, unsigned int i)
{
    return read_le16(r->tab + i * REC_SIZE + 4);
}

/*! @brief Read DATTEXT.offset for record @p i.

    @param[in] r Parsed REG.DAT.
    @param[in] i Record index.

    @return Offset of the first string byte inside the text block.
*/
static unsigned int dt_offset(const REGDAT *r, unsigned int i)
{
    return read_le16(r->tab + i * REC_SIZE + 6);
}

/* ------------------------------------------------------------------ */
/* Hash function (Windows 3.1 REG.DAT formula)                         */
/* ------------------------------------------------------------------ */

/*! @brief Compute the REG.DAT hash bucket for a string.

    Uses the Windows 3.1 formula: sum(toupper(c)) % hashsize.

    @param[in] s        NUL-terminated string.
    @param[in] hashsize Number of buckets.

    @return Bucket index in [0, hashsize).
*/
static unsigned int reg_hash(const char *s, unsigned int hashsize)
{
    unsigned int h = 0;
    while (*s) {
        h += (unsigned int)(unsigned char)toupper((unsigned char)*s);
        s++;
    }
    return h % hashsize;
}

/* ------------------------------------------------------------------ */
/* DATTEXT validation and string extraction                            */
/* ------------------------------------------------------------------ */

/*! @brief Validate a DATTEXT record.

    Checks the index range, offset+length against the text block, and
    the (2*index+1) prefix word. The string is NOT NUL-terminated in
    the text block; only the Length field delimits it.

    @param[in,out] r   Parsed REG.DAT.
    @param[in,out] s   Stats.
    @param[in]     idx DATTEXT index to validate.

    @return 1 if valid, 0 otherwise.

    @retval 1 The DATTEXT record is structurally valid.
    @retval 0 Bounds or prefix-word check failed.
*/
static int validate_dt(REGDAT *r, STATS *s, unsigned int idx)
{
    unsigned int len, off, prefix, expected;

    s->checks++;
    if (idx == 0 || idx >= r->dwEntries) {
        printf("  ERROR: DATTEXT index %u out of range [1,%u)\n",
               idx, r->dwEntries);
        s->errors++;
        return 0;
    }

    len = dt_length(r, idx);
    off = dt_offset(r, idx);

    if (off < 2 || off + len > r->sizeText) {
        printf("  ERROR: DATTEXT %u offset=%u len=%u outside text block "
               "(size=%u)\n", idx, off, len, r->sizeText);
        s->errors++;
        return 0;
    }

    prefix   = read_le16(r->text + off - 2);
    expected = (idx * 2 + 1) & 0xFFFF;
    if (prefix != expected) {
        printf("  ERROR: DATTEXT %u prefix word 0x%04X != 2*%u+1 (0x%04X)\n",
               idx, prefix, idx, expected);
        s->errors++;
        return 0;
    }

    return 1;
}

/*! @brief Copy the string referenced by a DATTEXT index into @p buf.

    The string is not NUL-terminated in the text block; @p buf is
    NUL-terminated here for convenience. Long strings are truncated to
    @p bufsz - 1 bytes.

    @param[in]  r     Parsed REG.DAT.
    @param[in]  idx   DATTEXT index.
    @param[out] buf   Output buffer.
    @param[in]  bufsz Size of @p buf in bytes.
*/
static void dt_string(REGDAT *r, unsigned int idx, char *buf, size_t bufsz)
{
    unsigned int len = dt_length(r, idx);
    unsigned int off = dt_offset(r, idx);

    if (len >= bufsz) len = (unsigned int)(bufsz - 1);
    memcpy(buf, r->text + off, len);
    buf[len] = 0;
}

/* ------------------------------------------------------------------ */
/* Hash bucket walk                                                    */
/* ------------------------------------------------------------------ */

/*! @brief Walk one bucket and validate every string in it.

    @param[in,out] r        Parsed REG.DAT.
    @param[in,out] s        Stats.
    @param[in]     bucket   Bucket index (1..hashsize).
    @param[out]    in_chain Bitmap; in_chain[i] set for every DATTEXT i
                            visited through this bucket.
    @param[in]     verbose  Verbose output flag.
*/
static void walk_bucket(REGDAT *r, STATS *s, unsigned int bucket,
                        unsigned char *in_chain, int verbose)
{
    unsigned int cur = read_le16(r->tab + bucket * REC_SIZE + 0);
    unsigned int steps = 0;
    char buf[512];

    if (cur == 0) return;

    if (verbose) printf("  bucket %2u (hash %2u):", bucket, bucket - 1);

    while (cur != 0 && cur != bucket) {
        s->checks++;

        if (cur <= r->hashsize) {
            printf("\n  ERROR: bucket %u chain enters bucket zone at index %u\n",
                   bucket, cur);
            s->errors++;
            return;
        }
        if (cur >= r->dwEntries) {
            printf("\n  ERROR: bucket %u chain reaches out-of-range index %u\n",
                   bucket, cur);
            s->errors++;
            return;
        }
        if (in_chain[cur]) {
            printf("\n  ERROR: bucket %u chain revisits index %u\n",
                   bucket, cur);
            s->errors++;
            return;
        }
        if (++steps > r->dwEntries) {
            printf("\n  ERROR: bucket %u chain longer than dwEntries\n", bucket);
            s->errors++;
            return;
        }
        in_chain[cur] = 1;

        if (!validate_dt(r, s, cur)) {
            printf("  (bucket %u, DATTEXT %u)\n", bucket, cur);
            return;
        }

        dt_string(r, cur, buf, sizeof(buf));

        if (reg_hash(buf, r->hashsize) + 1 != bucket) {
            printf("\n  ERROR: bucket %u contains string \"%s\" (DATTEXT %u) "
                   "which hashes to %u\n",
                   bucket, buf, cur, reg_hash(buf, r->hashsize));
            s->errors++;
        }

        if (verbose) printf(" %u", cur);

        cur = dt_index(r, cur);
    }

    if (verbose) {
        if (cur == bucket) printf(" (circular)");
        else               printf(" (linear)");
        printf("\n");
    }
}

/* ------------------------------------------------------------------ */
/* Tree walk                                                           */
/* ------------------------------------------------------------------ */

/*! @brief Walk the navigation tree from @p idx recursively.

    @param[in,out] r          Parsed REG.DAT.
    @param[in,out] s          Stats.
    @param[in]     idx        Starting dirent index.
    @param[out]    in_dirent  Bitmap marking dirents reached via Sub/Next.
    @param[out]    in_dt_tree Bitmap marking DATTEXTs referenced via
                              Name/Value.
    @param[in]     depth      Recursion depth (for verbose indentation).
    @param[in]     verbose    Verbose output flag.
*/
static void walk_tree(REGDAT *r, STATS *s, unsigned int idx,
                      unsigned char *in_dirent, unsigned char *in_dt_tree,
                      int depth, int verbose)
{
    unsigned int next, sub, name, value;
    char buf[512];

    while (idx != 0) {
        s->checks++;

        if (idx <= r->hashsize) {
            printf("  ERROR: dirent index %u is in the bucket zone\n", idx);
            s->errors++;
            return;
        }
        if (idx >= r->dwEntries) {
            printf("  ERROR: dirent index %u out of range\n", idx);
            s->errors++;
            return;
        }
        if (in_dirent[idx]) {
            printf("  ERROR: dirent loop at index %u\n", idx);
            s->errors++;
            return;
        }
        in_dirent[idx] = 1;

        next  = read_le16(r->tab + idx * REC_SIZE + 0);
        sub   = read_le16(r->tab + idx * REC_SIZE + 2);
        name  = read_le16(r->tab + idx * REC_SIZE + 4);
        value = read_le16(r->tab + idx * REC_SIZE + 6);

        if (verbose)
            printf("%*s[%u] Next=%u Sub=%u Name=%u Value=%u\n",
                   depth * 2, "", idx, next, sub, name, value);

        if (name != 0) {
            if (name >= r->dwEntries) {
                printf("  ERROR: dirent %u Name index %u out of range\n",
                       idx, name);
                s->errors++;
            } else if (validate_dt(r, s, name)) {
                in_dt_tree[name] = 1;
                dt_string(r, name, buf, sizeof(buf));
                if (verbose)
                    printf("%*s    Name=\"%s\"\n", depth * 2, "", buf);
            }
        }

        if (value != 0) {
            if (value >= r->dwEntries) {
                printf("  ERROR: dirent %u Value index %u out of range\n",
                       idx, value);
                s->errors++;
            } else if (validate_dt(r, s, value)) {
                in_dt_tree[value] = 1;
                dt_string(r, value, buf, sizeof(buf));
                if (verbose)
                    printf("%*s    Value=\"%s\"\n", depth * 2, "", buf);
            }
        }

        if (sub != 0) {
            walk_tree(r, s, sub, in_dirent, in_dt_tree, depth + 1, verbose);
        }

        idx = next;
    }
}

/* ------------------------------------------------------------------ */
/* Free list walk                                                      */
/* ------------------------------------------------------------------ */

/*! @brief Walk the free list starting from @p r->freeidx.

    @param[in,out] r       Parsed REG.DAT.
    @param[in,out] s       Stats.
    @param[out]    in_free Bitmap marking slots in the free list.
    @param[in]     verbose Verbose output flag.
*/
static void walk_free_list(REGDAT *r, STATS *s, unsigned char *in_free,
                           int verbose)
{
    unsigned int cur = r->freeidx;
    unsigned int count = 0;

    if (verbose) printf("  Free list:");

    while (cur != 0 && cur < r->dwEntries) {
        s->checks++;
        if (in_free[cur]) {
            printf("\n  ERROR: free list loop at index %u\n", cur);
            s->errors++;
            return;
        }
        in_free[cur] = 1;
        count++;
        if (verbose) printf(" %u", cur);
        cur = read_le16(r->tab + cur * REC_SIZE);
    }

    if (verbose) printf("  (%u slot%s)\n", count, count == 1 ? "" : "s");
}

/* ------------------------------------------------------------------ */
/* .classes known-vector check                                         */
/* ------------------------------------------------------------------ */

/*! @brief Verify that @c .classes is present and hashes to 0x11.

    Searches all DATTEXT records reached through hash chains (not the
    tree) so that the check works even if the tree walk failed.

    @param[in] r        Parsed REG.DAT.
    @param[in] in_chain Bitmap of DATTEXT records seen through chains.
    @param[in] verbose  Verbose output flag.
*/
static void check_known_vector(REGDAT *r, unsigned char *in_chain,
                               int verbose)
{
    unsigned int i;
    char buf[512];
    int found = 0;

    for (i = 1; i < r->dwEntries; i++) {
        unsigned int h;

        if (!in_chain[i]) continue;
        dt_string(r, i, buf, sizeof(buf));
        if (strcmp(buf, KNOWN_STRING) != 0) continue;

        h = reg_hash(buf, r->hashsize);
        if (verbose)
            printf("  \"%s\" found at DATTEXT %u, hash=%u (0x%X)\n",
                   buf, i, h, h);

        if (h == KNOWN_HASH && r->hashsize == KNOWN_HASHSIZE) {
            printf("  \"%s\" hash == 0x%X (matches note.txt)\n",
                   KNOWN_STRING, KNOWN_HASH);
        } else {
            printf("  WARNING: \"%s\" hash == 0x%X with hashsize %u, "
                   "expected 0x%X with %u\n",
                   KNOWN_STRING, h, r->hashsize,
                   KNOWN_HASH, KNOWN_HASHSIZE);
        }
        found = 1;
        break;
    }

    if (!found) {
        printf("  WARNING: \"%s\" not found in any chain\n", KNOWN_STRING);
    }
}

/* ------------------------------------------------------------------ */
/* Main                                                                */
/* ------------------------------------------------------------------ */

/*! @brief REG.DAT parser and validator entry point.

    @param[in] argc Argument count.
    @param[in] argv Argument vector.

    @return 0 on success, 1 on failure.

    @retval 0 All checks passed.
    @retval 1 A parse or validation error was detected.
*/
int main(int argc, char **argv)
{
    const char *path = "REG.DAT";
    int verbose = 0;
    int quiet   = 0;
    int i;

    REGDAT r;
    STATS  s;
    unsigned char *in_chain = NULL, *in_dirent = NULL;
    unsigned char *in_dt_tree = NULL, *in_free = NULL;
    unsigned int b;
    int ok = 0;

    for (i = 1; i < argc; i++) {
        if      (!strcmp(argv[i], "-v") || !strcmp(argv[i], "--verbose")) verbose = 1;
        else if (!strcmp(argv[i], "-q") || !strcmp(argv[i], "--quiet"))   quiet = 1;
        else if (!strcmp(argv[i], "-h") || !strcmp(argv[i], "--help")) {
            printf("Usage: %s [file] [-v] [-q]\n", argv[0]);
            return 0;
        }
        else path = argv[i];
    }

    memset(&r, 0, sizeof(r));
    memset(&s, 0, sizeof(s));

    if (!quiet) {
        printf("REG.DAT parser and validator\n");
        printf("============================\n\n");
        printf("File: %s\n", path);
    }

    r.raw = read_file(path, &r.size);
    if (!r.raw) {
        printf("ERROR: cannot read \"%s\"\n", path);
        return 1;
    }

    if (!quiet) printf("Size: %u bytes\n\n", r.size);

    if (!parse_header(&r, &s, verbose)) {
        printf("\nFAILED: header invalid\n");
        goto done;
    }

    s.checks += 2;
    if (r.unk1 != r.unk2) {
        printf("ERROR: unk1 (%u) != unk2 (%u)\n", r.unk1, r.unk2);
        s.errors++;
    }
    if (r.unk1 + r.dwEntries * REC_SIZE != r.offText) {
        printf("ERROR: unk1 + dwEntries*8 (%u) != offsetDataBlock (%u)\n",
               r.unk1 + r.dwEntries * REC_SIZE, r.offText);
        s.errors++;
    }
    if (r.offText + r.sizeText != r.size) {
        printf("WARNING: offsetDataBlock + sizeDataBlock (%u) != file size "
               "(%u); file may contain trailing padding\n",
               r.offText + r.sizeText, r.size);
    }

    in_chain   = (unsigned char *)calloc(r.dwEntries, 1);
    in_dirent  = (unsigned char *)calloc(r.dwEntries, 1);
    in_dt_tree = (unsigned char *)calloc(r.dwEntries, 1);
    in_free    = (unsigned char *)calloc(r.dwEntries, 1);
    if (!in_chain || !in_dirent || !in_dt_tree || !in_free) {
        printf("ERROR: out of memory\n");
        goto done;
    }

    if (verbose) printf("\nHash buckets (1..%u):\n", r.hashsize);
    for (b = 1; b <= r.hashsize; b++) {
        walk_bucket(&r, &s, b, in_chain, verbose);
    }

    {
        unsigned int root_sub = read_le16(r.tab + 0 * REC_SIZE + 2);

        if (verbose)
            printf("\nTree (from tab[0].Sub = %u):\n", root_sub);

        if (root_sub != 0 && root_sub >= r.dwEntries) {
            printf("  ERROR: tab[0].Sub (%u) out of range\n", root_sub);
            s.errors++;
        } else if (root_sub != 0) {
            walk_tree(&r, &s, root_sub, in_dirent, in_dt_tree, 1, verbose);
        }
    }

    {
        unsigned int only_chain = 0, only_tree = 0;
        for (i = 1; i < (int)r.dwEntries; i++) {
            if (in_chain[i]   && !in_dt_tree[i]) only_chain++;
            if (in_dt_tree[i] && !in_chain[i])   only_tree++;
        }
        if (only_chain || only_tree) {
            printf("  ERROR: DATTEXT sets differ between chains and tree "
                   "(only-in-chain=%u, only-in-tree=%u)\n",
                   only_chain, only_tree);
            s.errors++;
        } else if (verbose) {
            printf("  OK: every DATTEXT in a chain is referenced from the tree "
                   "and vice versa\n");
        }
        s.checks++;
    }

    if (verbose) printf("\nFree list:\n");
    walk_free_list(&r, &s, in_free, verbose);

    if (!quiet) {
        printf("\nKnown-vector check:\n");
        check_known_vector(&r, in_chain, verbose);
    }

    {
        unsigned int bad = 0;
        for (i = (int)r.hashsize + 1; i < (int)r.dwEntries; i++) {
            int roles = (in_dirent[i] != 0) + (in_chain[i] != 0) + (in_free[i] != 0);
            if (roles > 1) {
                printf("  ERROR: record %u has conflicting roles "
                       "(dirent=%d chain=%d free=%d)\n",
                       i, in_dirent[i] ? 1 : 0, in_chain[i] ? 1 : 0,
                       in_free[i] ? 1 : 0);
                bad++;
            }
        }
        if (bad) s.errors += bad;
        s.checks++;
    }

    {
        unsigned int dirents = 0, dts = 0, frees = 0, unused = 0;

        for (i = (int)r.hashsize + 1; i < (int)r.dwEntries; i++) {
            if (in_dirent[i]) dirents++;
            if (in_chain[i])  dts++;
            if (in_free[i])   frees++;
            if (!in_dirent[i] && !in_chain[i] && !in_free[i]) unused++;
        }

        printf("\nSummary:\n");
        printf("  Total records:      %u\n", r.dwEntries);
        printf("  Root:               1\n");
        printf("  Buckets:            %u\n", r.hashsize);
        printf("  Dirents:            %u\n", dirents);
        printf("  DATTEXTs:           %u\n", dts);
        printf("  Free:               %u\n", frees);
        if (unused)
            printf("  Unaccounted:        %u\n", unused);
        printf("  Checks:             %u\n", s.checks);
        printf("  Errors:             %u\n", s.errors);

        if (s.errors == 0 && unused == 0) {
            printf("\nPASS\n");
            ok = 1;
        } else if (s.errors == 0) {
            printf("\nPASS (with %u unaccounted records)\n", unused);
            ok = 1;
        } else {
            printf("\nFAIL\n");
        }
    }

done:
    free(in_chain);
    free(in_dirent);
    free(in_dt_tree);
    free(in_free);
    free(r.raw);
    return ok ? 0 : 1;
}
