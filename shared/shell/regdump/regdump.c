/*
 * 				REG.DAT dumper
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

/*! @file regdump.c
    @brief Host-side REG.DAT dumper.

    Reads a Windows 3.1 REG.DAT and dumps its logical content (keys and
    default values), optionally with internal structure (hash buckets,
    free list, dirent indexes).

    Output is always HKEY_CLASSES_ROOT as the root display name,
    because Windows 3.1 maps HKEY_CLASSES_ROOT to the ".classes" root
    entry of the file. Default values are shown as `= "value"` (empty
    string when the dirent has no Value or an empty one).

    Usage:  regdump [file] [-v] [-b] [-f] [-q]

        file    path to REG.DAT (default: REG.DAT)
        -v      verbose: prefix each key with dirent indexes
        -b      also dump hash buckets
        -f      also dump free list
        -q      quiet: only the key/value lines

    Exit code: 0 on success, 1 on any read or parse failure.
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* ------------------------------------------------------------------ */
/* Little-endian readers                                               */
/* ------------------------------------------------------------------ */

/*! @brief Read a little-endian 16-bit word.

    @param[in] p Pointer to 2 bytes.

    @return The value as unsigned int.
*/
static unsigned int read_le16(const unsigned char *p)
{
    return (unsigned int)p[0] | ((unsigned int)p[1] << 8);
}

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

/* ------------------------------------------------------------------ */
/* REG.DAT header field offsets                                        */
/* ------------------------------------------------------------------ */

/*! @def HDR_SIG
    @brief Header offset of the signature field. */
#define HDR_SIG        0

/*! @def HDR_UNK1
    @brief Header offset of unk1. */
#define HDR_UNK1       8

/*! @def HDR_UNK2
    @brief Header offset of unk2. */
#define HDR_UNK2      12

/*! @def HDR_DWENTRIES
    @brief Header offset of the entry count. */
#define HDR_DWENTRIES 16

/*! @def HDR_OFFTEXT
    @brief Header offset of the text-block offset. */
#define HDR_OFFTEXT   20

/*! @def HDR_SIZETEXT
    @brief Header offset of the text-block size. */
#define HDR_SIZETEXT  24

/*! @def HDR_HASHSIZE
    @brief Header offset of the hash-bucket count. */
#define HDR_HASHSIZE  28

/*! @def HDR_FREEIDX
    @brief Header offset of the first free entry. */
#define HDR_FREEIDX   30

/*! @def HDR_SIZE
    @brief Total header size in bytes. */
#define HDR_SIZE      32

/*! @def REC_SIZE
    @brief Size of one entry-table record in bytes. */
#define REC_SIZE       8

/*! @def PATH_MAX_LEN
    @brief Maximum length of a display path, including terminator. */
#define PATH_MAX_LEN  8192

/* ------------------------------------------------------------------ */
/* Parsed view                                                         */
/* ------------------------------------------------------------------ */

/*! @brief Parsed view of a REG.DAT file. */
typedef struct {
    unsigned char *raw;        /*!< Raw file contents. */
    unsigned int   size;       /*!< Total file size. */

    unsigned int   unk1;       /*!< Header field at 0x08. */
    unsigned int   dwEntries;  /*!< Header field at 0x10. */
    unsigned int   offText;    /*!< Header field at 0x14. */
    unsigned int   sizeText;   /*!< Header field at 0x18. */
    unsigned int   hashsize;   /*!< Header field at 0x1C. */
    unsigned int   freeidx;    /*!< Header field at 0x1E. */

    const unsigned char *tab;  /*!< Pointer to the entry table. */
    const unsigned char *text; /*!< Pointer to the text block. */
} REGDAT;

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
/* Header parse                                                        */
/* ------------------------------------------------------------------ */

/*! @brief Parse and minimally validate the REG.DAT header.

    @param[in,out] r Parsed view (fields filled in on success).

    @return 1 if the header is valid, 0 otherwise.

    @retval 1 The header is valid and the file layout is sane.
    @retval 0 The header is invalid.
*/
static int parse_header(REGDAT *r)
{
    if (r->size < HDR_SIZE) {
        fprintf(stderr, "ERROR: file too short for REG.DAT header\n");
        return 0;
    }
    if (memcmp(r->raw + HDR_SIG, "SHCC3.10", 8) != 0) {
        fprintf(stderr, "ERROR: bad signature\n");
        return 0;
    }

    r->unk1      = read_le32(r->raw + HDR_UNK1);
    r->dwEntries = read_le32(r->raw + HDR_DWENTRIES);
    r->offText   = read_le32(r->raw + HDR_OFFTEXT);
    r->sizeText  = read_le32(r->raw + HDR_SIZETEXT);
    r->hashsize  = read_le16(r->raw + HDR_HASHSIZE);
    r->freeidx   = read_le16(r->raw + HDR_FREEIDX);

    if (r->dwEntries == 0 || r->hashsize == 0) {
        fprintf(stderr, "ERROR: dwEntries or hashsize is zero\n");
        return 0;
    }
    if (r->unk1 < HDR_SIZE || r->unk1 > r->size) {
        fprintf(stderr, "ERROR: unk1 outside file\n");
        return 0;
    }
    if (r->dwEntries > (r->size - r->unk1) / REC_SIZE) {
        fprintf(stderr, "ERROR: entry table past end of file\n");
        return 0;
    }
    if (r->offText > r->size || r->sizeText > r->size - r->offText) {
        fprintf(stderr, "ERROR: text block past end of file\n");
        return 0;
    }

    r->tab  = r->raw + r->unk1;
    r->text = r->raw + r->offText;
    return 1;
}

/* ------------------------------------------------------------------ */
/* DATTEXT accessors                                                   */
/* ------------------------------------------------------------------ */

/*! @brief Read DATTEXT.Length for record @p i.

    @param[in] r Parsed REG.DAT.
    @param[in] i Record index.

    @return String length in bytes.
*/
static unsigned int dt_length(REGDAT *r, unsigned int i)
{
    return read_le16(r->tab + i * REC_SIZE + 4);
}

/*! @brief Read DATTEXT.offset for record @p i.

    @param[in] r Parsed REG.DAT.
    @param[in] i Record index.

    @return Offset of the string inside the text block.
*/
static unsigned int dt_offset(REGDAT *r, unsigned int i)
{
    return read_le16(r->tab + i * REC_SIZE + 6);
}

/*! @brief Copy a DATTEXT string into @p buf (NUL-terminated).

    The string is not NUL-terminated in the text block; only Length
    delimits it. Strings longer than @p bufsz - 1 are truncated.

    @param[in]  r     Parsed REG.DAT.
    @param[in]  idx   DATTEXT index (must be < dwEntries).
    @param[out] buf   Output buffer.
    @param[in]  bufsz Size of @p buf in bytes.
*/
static void dt_string(REGDAT *r, unsigned int idx, char *buf, size_t bufsz)
{
    unsigned int len = dt_length(r, idx);
    unsigned int off = dt_offset(r, idx);

    if (len >= bufsz) len = (unsigned int)(bufsz - 1);
    if (off + len > r->sizeText) { buf[0] = 0; return; }
    memcpy(buf, r->text + off, len);
    buf[len] = 0;
}

/* ------------------------------------------------------------------ */
/* Dump state                                                          */
/* ------------------------------------------------------------------ */

/*! @brief Current display path (shared across recursion). */
static char g_path[PATH_MAX_LEN];
/*! @brief Current length of @ref g_path. */
static size_t g_path_len = 0;
/*! @brief Number of keys dumped so far. */
static int g_key_count = 0;
/*! @brief Bitmap of dirents already visited (loop detection). */
static unsigned char *g_visited = NULL;

/* ------------------------------------------------------------------ */
/* Tree walk                                                           */
/* ------------------------------------------------------------------ */

/*! @brief Recursively dump a dirent and its siblings/children.

    Walks the Next chain at the current level, printing each key, then
    recurses into Sub. Updates @ref g_path incrementally and restores it
    before each sibling.

    @param[in] r       Parsed REG.DAT.
    @param[in] idx     Starting dirent index (0 = no key).
    @param[in] depth   Recursion depth (0 = root level).
    @param[in] verbose When non-zero, prefix each key with dirent indexes.
*/
static void dump_tree(REGDAT *r, unsigned int idx, int depth, int verbose)
{
    size_t saved_len = g_path_len;
    char name[512], value[512];

    while (idx != 0) {
        unsigned int next, sub, name_idx, value_idx;

        if (idx >= r->dwEntries) {
            fprintf(stderr, "ERROR: dirent index %u out of range\n", idx);
            return;
        }
        if (g_visited[idx]) {
            fprintf(stderr, "ERROR: dirent loop at index %u\n", idx);
            return;
        }
        g_visited[idx] = 1;

        next      = read_le16(r->tab + idx * REC_SIZE + 0);
        sub       = read_le16(r->tab + idx * REC_SIZE + 2);
        name_idx  = read_le16(r->tab + idx * REC_SIZE + 4);
        value_idx = read_le16(r->tab + idx * REC_SIZE + 6);

        if (name_idx != 0 && name_idx < r->dwEntries) {
            dt_string(r, name_idx, name, sizeof(name));
        } else {
            name[0] = 0;
        }
        if (value_idx != 0 && value_idx < r->dwEntries) {
            dt_string(r, value_idx, value, sizeof(value));
        } else {
            value[0] = 0;
        }

        if (depth == 0) {
            strncpy(g_path, "HKEY_CLASSES_ROOT", sizeof(g_path) - 1);
            g_path[sizeof(g_path) - 1] = 0;
            g_path_len = strlen(g_path);
        } else if (name[0]) {
            size_t nlen = strlen(name);
            if (g_path_len + 1 + nlen >= sizeof(g_path)) {
                nlen = sizeof(g_path) - g_path_len - 2;
            }
            if (g_path_len < sizeof(g_path) - 1) {
                g_path[g_path_len++] = '\\';
            }
            memcpy(g_path + g_path_len, name, nlen);
            g_path_len += nlen;
            g_path[g_path_len] = 0;
        } else {
            if (g_path_len + 10 < sizeof(g_path)) {
                g_path[g_path_len++] = '\\';
                memcpy(g_path + g_path_len, "(default)", 9);
                g_path_len += 9;
                g_path[g_path_len] = 0;
            }
        }

        if (verbose) {
            printf("[idx=%3u next=%3u sub=%3u name=%3u value=%3u] ",
                   idx, next, sub, name_idx, value_idx);
        }
        if (value[0]) {
            printf("%s = \"%s\"\n", g_path, value);
        } else {
            printf("%s = \"\"\n", g_path);
        }
        g_key_count++;

        if (sub != 0) {
            dump_tree(r, sub, depth + 1, verbose);
        }

        g_path_len = saved_len;
        g_path[g_path_len] = 0;

        idx = next;
    }
}

/* ------------------------------------------------------------------ */
/* Bucket dump                                                         */
/* ------------------------------------------------------------------ */

/*! @brief Dump all non-empty hash buckets and their chains.

    @param[in] r Parsed REG.DAT.
*/
static void dump_buckets(REGDAT *r)
{
    unsigned int b;

    printf("\nHash buckets (1..%u):\n", r->hashsize);
    for (b = 1; b <= r->hashsize; b++) {
        unsigned int cur = read_le16(r->tab + b * REC_SIZE);
        if (cur == 0) continue;

        printf("  bucket %2u (hash %2u):", b, b - 1);
        while (cur != 0 && cur != b) {
            char buf[512];
            dt_string(r, cur, buf, sizeof(buf));
            printf(" %u(\"%s\")", cur, buf);
            cur = read_le16(r->tab + cur * REC_SIZE);
        }
        printf("\n");
    }
}

/* ------------------------------------------------------------------ */
/* Free list dump                                                      */
/* ------------------------------------------------------------------ */

/*! @brief Dump the free list starting from @p r->freeidx.

    @param[in] r Parsed REG.DAT.
*/
static void dump_free_list(REGDAT *r)
{
    unsigned int cur = r->freeidx;
    unsigned int count = 0;

    printf("\nFree list (from %u):", cur);
    while (cur != 0 && cur < r->dwEntries) {
        printf(" %u", cur);
        cur = read_le16(r->tab + cur * REC_SIZE);
        count++;
        if (count > r->dwEntries) {
            printf(" ... (loop)");
            break;
        }
    }
    printf("\n");
}

/* ------------------------------------------------------------------ */
/* Main                                                                */
/* ------------------------------------------------------------------ */

/*! @brief REG.DAT dumper entry point.

    @param[in] argc Argument count.
    @param[in] argv Argument vector.

    @return 0 on success, 1 on failure.

    @retval 0 Dump produced without errors.
    @retval 1 File missing, malformed, or memory allocation failed.
*/
int main(int argc, char **argv)
{
    const char *path = "REG.DAT";
    int verbose = 0;
    int do_buckets = 0;
    int do_free = 0;
    int quiet = 0;
    int i;

    REGDAT r;
    unsigned int root_sub;

    for (i = 1; i < argc; i++) {
        if      (!strcmp(argv[i], "-v") || !strcmp(argv[i], "--verbose")) verbose = 1;
        else if (!strcmp(argv[i], "-b") || !strcmp(argv[i], "--buckets")) do_buckets = 1;
        else if (!strcmp(argv[i], "-f") || !strcmp(argv[i], "--free"))    do_free = 1;
        else if (!strcmp(argv[i], "-q") || !strcmp(argv[i], "--quiet"))   quiet = 1;
        else if (!strcmp(argv[i], "-h") || !strcmp(argv[i], "--help")) {
            printf("Usage: %s [file] [-v] [-b] [-f] [-q]\n", argv[0]);
            return 0;
        }
        else path = argv[i];
    }

    memset(&r, 0, sizeof(r));

    r.raw = read_file(path, &r.size);
    if (!r.raw) {
        fprintf(stderr, "ERROR: cannot read \"%s\"\n", path);
        return 1;
    }

    if (!parse_header(&r)) {
        free(r.raw);
        return 1;
    }

    if (!quiet) {
        printf("REG.DAT dumper\n");
        printf("==============\n\n");
        printf("File:     %s\n", path);
        printf("Size:     %u bytes\n", r.size);
        printf("Entries:  %u\n", r.dwEntries);
        printf("Buckets:  %u\n", r.hashsize);
        printf("Free idx: %u\n\n", r.freeidx);
    }

    g_visited = (unsigned char *)calloc(r.dwEntries, 1);
    if (!g_visited) {
        fprintf(stderr, "ERROR: out of memory\n");
        free(r.raw);
        return 1;
    }

    g_path_len = 0;
    g_path[0] = 0;
    g_key_count = 0;

    root_sub = read_le16(r.tab + 0 * REC_SIZE + 2);

    if (root_sub != 0 && root_sub < r.dwEntries) {
        dump_tree(&r, root_sub, 0, verbose);
    } else {
        fprintf(stderr, "WARNING: empty or invalid root (tab[0].Sub = %u)\n",
                root_sub);
    }

    if (do_buckets) dump_buckets(&r);
    if (do_free)    dump_free_list(&r);

    if (!quiet) {
        printf("\n%u key(s) dumped\n", g_key_count);
    }

    free(g_visited);
    free(r.raw);
    return 0;
}
