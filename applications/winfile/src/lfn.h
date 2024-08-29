/********************************************************************

   lfn.h

   declaration of lfn aware functions

   Copyright (c) Microsoft Corporation. All rights reserved.
   Licensed under the MIT License.

********************************************************************/

#define CCHMAXFILE      MAXPATHLEN         // max size of a long name
#define CCHMAXPATHCOMP  256
#define LFNCANON_MASK           1
#define PATHLEN1_1 13

#define FILE_83_CI  0
#define FILE_83_CS  1
#define FILE_LONG   2

#define ERROR_OOM   8

// we need to add an extra field to distinguish DOS vs. LFNs

typedef struct {
   HANDLE hFindFile;           // handle returned by FindFirstFile()
   DWORD dwAttrFilter;         // search attribute mask.
   DWORD err;                  // error info if failure.
   struct find_t fd;			//WIN32_FIND_DATA fd;         // FindFirstFile() data strucrure;
   int   nSpaceLeft;           // Space left for deeper paths
} LFNDTA, *LPLFNDTA, * PLFNDTA;

VOID  LFNInit( VOID );
VOID  InvalidateVolTypes( VOID );

DWORD  GetNameType(LPSTR);
BOOL  IsLFN(LPSTR pName);

BOOL  WFFindFirst(LPLFNDTA lpFind, LPSTR lpName, DWORD dwAttrFilter);
BOOL  WFFindNext(LPLFNDTA);
BOOL  WFFindClose(LPLFNDTA);


DWORD  I_LFNCanon( WORD /*USHORT*/ CanonType, LPSTR InFile, LPSTR OutFile );
DWORD  LFNParse(LPSTR,LPSTR,LPSTR);
WORD I_LFNEditName( LPSTR lpSrc, LPSTR lpEd, LPSTR lpRes, int iResBufSize );

BOOL  WFIsDir(LPSTR);
BOOL  LFNMergePath(LPSTR,LPSTR);

BOOL  IsLFNSelected(VOID);
