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

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <direct.h>
#include <dos.h>
#define MAX_PATH FILENAME_MAX

#include <windows.h>
#include <shellapi.h>
 
#define SAB_OKAY			IDOK
#define SAB_ABOUT			1000
#define SAB_TEXT			1001
#define SAB_ICON			1002

#define SAB_USER			1003
#define SAB_VERSION			1005

#define SAB_WINDOW			1006 
#define SAB_SYSTEM			1007

#define SAB_HOST			1008
#define SAB_TERM			1009

#define IDD_SHELLABOUT                  4001

void lmemcpy(void FAR * s1, void FAR * s2, unsigned length);
int lstrnicmp(char FAR *s1, const char FAR *s2, int n);
char FAR *lstrchr(const char FAR *s, int c);
void lmemcpy(void FAR * s1, void FAR * s2, unsigned length);
void FAR * lmemset (void FAR *start, int c, int len);
//int toupper (int c);

#define GET_WM_COMMAND_ID(wp, lp)                   (wp)

typedef struct {
	LPSTR	lpszCaption;
	LPSTR	lpszText;
	HICON	hIcon;
} SHELLABOUTDATA, FAR * LPSHELLABOUTDATA;

#define GlobalPtrHandle(lp) \
  ((HGLOBAL)LOWORD(GlobalHandle(SELECTOROF(lp))))

#define     GlobalUnlockPtr(lp)      \
                GlobalUnlock(GlobalPtrHandle(lp))

#define GlobalFreePtr(lp) \
  (GlobalUnlockPtr(lp),(BOOL)GlobalFree(GlobalPtrHandle(lp)))

#define GlobalAllocPtr(flags, cb) \
  (GlobalLock(GlobalAlloc((flags), (cb))))

extern  unsigned short          GetDS( void );
#pragma aux GetDS               = \
        "mov    ax,ds"          \
        value                   [ax];

typedef struct
{
	WORD	size;
	HANDLE	entries[1];
} ATOMTABLE;
typedef ATOMTABLE FAR *LPATOMTABLE;


ATOM WINAPI DeleteAtomEx(LPATOMTABLE atomtable, ATOM atom );
ATOM WINAPI FindAtomEx(LPATOMTABLE atomtable, LPCSTR str );
UINT WINAPI GetAtomNameEx(LPATOMTABLE atomtable, ATOM atom, LPSTR buffer, int count );
ATOM WINAPI AddAtomEx(LPATOMTABLE atomtable, LPCSTR str );

extern  unsigned short          GetDS( void );
#pragma aux GetDS               = \
        "mov    ax,ds"          \
        value                   [ax];

#define DEFAULT_ATOMTABLE_SIZE    37
#define MAX_ATOM_LEN              255
#define MAXINTATOM          0xc000

#define ATOMTOHANDLE(atom)        ((HANDLE)(atom) << 2)
#define HANDLETOATOM(handle)      ((ATOM)(0xc000 | ((handle) >> 2)))


typedef struct
{
	HANDLE	next;
	WORD	refCount;
	BYTE	length;
	char	str[1];
} ATOMENTRY, FAR * LPATOMENTRY;

#ifndef	WM_DROPOBJECT
#define	WM_DROPOBJECT		0x022a
#endif

#ifndef	WM_QUERYDROPOBJECT
#define	WM_QUERYDROPOBJECT	0x022b
#endif

#ifndef	WM_BEGINDRAG
#define	WM_BEGINDRAG		0x022c
#endif

#ifndef	WM_DRAGLOOP
#define	WM_DRAGLOOP		0x022d
#endif

#ifndef	WM_DRAGSELECT
#define	WM_DRAGSELECT		0x022e
#endif

#ifndef	WM_DRAGMOVE
#define	WM_DRAGMOVE		0x022e
#endif

typedef struct tagDROPFILESTRUCT {
    WORD	wSize;
    POINT	ptMousePos;
    BOOL	fInNonClientArea;
} DROPFILESTRUCT, FAR *LPDROPFILESTRUCT;

#define offsetof(s,m)       (size_t)&(((s*)NULL)->m)
#define FIELD_OFFSET(type, field) ((LONG)offsetof(type, field))

BOOL CALLBACK AboutDlgProc(HWND, UINT, WPARAM, LPARAM);

static HINSTANCE hInst = 0;

#define AnsiUpperChar(c) ((char)AnsiUpper((LPSTR)(unsigned char)(c)))

#define SE_ERR_FNF 3
