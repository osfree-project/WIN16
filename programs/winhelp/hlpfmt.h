/*
 * Help Viewer
 *
 * Copyright    2023 Yuri Prokushev
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
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 *  Stuctures of HLP file format. Heavely based on documentation at
 *  https://www.oocities.org/mwinterhoff/helpdeco.htm
 *  https://entropymine.wordpress.com/2020/02/06/notes-on-winhelp-format-part-1/
 */

#include <windows.h>

#pragma pack( push, 1 )

/*
 * Help file consist of header and subfiles.
 */

typedef struct                /* structure at beginning of help file */
{
    DWORD Magic;              /* 0  0x00035F3F */
    DWORD DirectoryStart;     /* 4  offset of FILEHEADER of internal direcory */
    DWORD FreeChainStart;     /* 8  offset of FILEHEADER or -1L */
    DWORD EntireFileSize;     /* 12 size of entire help file in bytes */
} HELPHEADER;
typedef HELPHEADER far * LPHELPHEADER;

typedef struct FILEHEADER     /* structure at FileOffset of each internal file */
{
    DWORD ReservedSpace;      /* 0  reserved space in help file incl. FILEHEADER */
    DWORD UsedSpace;          /* 4  used space in help file excl. FILEHEADER */
    BYTE  FileFlags;          /* 8  normally 4 */
} FILEHEADER;

typedef struct BTREEHEADER    /* structure after FILEHEADER of each Btree */
{
    WORD  Magic;              /* 0  0x293B */
    WORD  Flags;              /* 2  bit 0x0002 always 1, bit 0x0400 1 if direcory */
    WORD  PageSize;           /* 4  0x0400=1k if directory, 0x0800=2k else */
    BYTE  Structure[16];      /* 20 string describing structure of data */
    WORD  MustBeZero;         /* 22  0 */
    WORD  PageSplits;         /* 24  number of page splits Btree has suffered */
    WORD  RootPage;           /* 26  page number of Btree root page */
    WORD  MustBeNegOne;       /* 28 0xFFFF */
    WORD  TotalPages;         /* 30 number of Btree pages */
    WORD  NLevels;            /* 32 number of levels of Btree */
    DWORD TotalBtreeEntries;  /* 34 number of entries in Btree */
} BTREEHEADER;
typedef BTREEHEADER far * LPBTREEHEADER;

typedef struct BTREEINDEXHEADER /* structure at beginning of every index-page */
{
    WORD Unknown;           /* 0  sorry, no ID to identify an index-page */
    WORD NEntries;          /* 2  number of entries in this index-page */
    WORD PreviousPage;      /* 4  page number of previous page */
} BTREEINDEXHEADER;

typedef struct BTREENODEHEADER /* structure at beginning of every leaf-page */
{
    WORD Unknown;  /* Sorry, no ID to identify a leaf-page */
    WORD NEntries;          /* number of entires in this leaf-page */
    WORD PreviousPage;      /* page number of preceeding leaf-page or -1 */
    WORD NextPage;          /* page number of next leaf-page or -1 */
} BTREENODEHEADER;

#pragma pack( pop )
