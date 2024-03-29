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

/* Global Data */
typedef struct tagGLOBALS {
	HINSTANCE	hInstance;		// SHELL.EXE Instance
	HGLOBAL		Registry;		// Registry Header Handle
	HGLOBAL		EntryTable;		// Registry Entries Table
	HGLOBAL		StringTable;		// Registry String Table
} GLOBALS;

extern GLOBALS Globals;

/* About Dialog */
#define IDD_SHELLABOUT                  100
 
#define SAB_OKAY			IDOK
#define SAB_ABOUT			1000

#define SAB_ICON			111
#define SAB_VERSION			112
#define SAB_TEXT			115

#define SAB_USER			1003

#define SAB_WINDOW			1006 
#define SAB_SYSTEM			1007

#define SAB_HOST			1008
#define SAB_TERM			1009

/* String resourses */
#define	IDS_WINDOWS		100
#define	IDS_PROGRAMS		101
#define	IDS_PROGRAMS_VALUE	102
#define	IDS_EXTENSIONS		103
#define	IDS_OPEN		104
#define IDS_REGISTRY		208
#define IDS_REALMODE		209
#define IDS_REALMODELEMS	210
#define IDS_REALMODESEMS	211
#define IDS_STANDARDMODE	212
#define IDS_ENHANCEDMODE	213
#define	IDS_SYSTEMRESOURCES	215
#define IDS_VERSION		216
#define IDS_DEBUG		217
#define IDS_FREE		218
#define IDS_FREEEMS		219
#define IDS_FREEP		220
#define IDS_FILENOTFOUND	223
#define IDS_FOUND		224
#define IDS_NOTFOUND		225
#define IDS_NOCOMMDLG		226
#define IDS_REGCLOSE		227

void lmemcpy(void FAR * s1, void FAR * s2, unsigned length);
int lstrnicmp(char FAR *s1, const char FAR *s2, int n);
char FAR *lstrchr(const char FAR *s, int c);
void lmemcpy(void FAR * s1, void FAR * s2, unsigned length);
void FAR * lmemset (void FAR *start, int c, int len);

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


#define AnsiUpperChar(c) ((char)AnsiUpper((LPSTR)(unsigned char)(c)))

#define SE_ERR_FNF 3

#define RSH_DEREGISTER          0
#define RSH_REGISTER            1
#define RSH_REGISTER_PROGMAN    2
#define RSH_REGISTER_TASKMAN    3

#pragma pack(push, 1)

#define IMAGE_DOS_SIGNATURE     0x5A4D
#define IMAGE_OS2_SIGNATURE     0x454E

/* Header shared by DOS, Win16, Win32, and Win64 executables */
typedef struct _IMAGE_DOS_HEADER {
    WORD    e_magic;
    WORD    e_cblp;
    WORD    e_cp;
    WORD    e_crlc;
    WORD    e_cparhdr;
    WORD    e_minalloc;
    WORD    e_maxalloc;
    WORD    e_ss;
    WORD    e_sp;
    WORD    e_csum;
    WORD    e_ip;
    WORD    e_cs;
    WORD    e_lfarlc;
    WORD    e_ovno;
    WORD    e_res[4];
    WORD    e_oemid;
    WORD    e_oeminfo;
    WORD    e_res2[10];
    LONG    e_lfanew;
} IMAGE_DOS_HEADER;
typedef IMAGE_DOS_HEADER FAR * LPIMAGE_DOS_HEADER;

/* Header for OS/2 executables */
typedef struct _IMAGE_OS2_HEADER {
    WORD    ne_magic;
    char    ne_ver;
    char    ne_rev;
    WORD    ne_enttab;
    WORD    ne_cbenttab;
    LONG    ne_crc;
    WORD    ne_flags;
    WORD    ne_autodata;
    WORD    ne_heap;
    WORD    ne_stack;
    LONG    ne_csip;
    LONG    ne_sssp;
    WORD    ne_cseg;
    WORD    ne_cmod;
    WORD    ne_cbnrestab;
    WORD    ne_segtab;
    WORD    ne_rsrctab;
    WORD    ne_restab;
    WORD    ne_modtab;
    WORD    ne_imptab;
    LONG    ne_nrestab;
    WORD    ne_cmovent;
    WORD    ne_align;
    WORD    ne_cres;
    BYTE    ne_exetyp;
    BYTE    ne_flagsothers;
    WORD    ne_pretthunks;
    WORD    ne_psegrefbytes;
    WORD    ne_swaparea;
    WORD    ne_expver;
} IMAGE_OS2_HEADER;
typedef IMAGE_OS2_HEADER FAR * LPIMAGE_OS2_HEADER;

typedef struct
{
    BYTE        bWidth;          /* Width, in pixels, of the image	*/
    BYTE        bHeight;         /* Height, in pixels, of the image	*/
    BYTE        bColorCount;     /* Number of colors in image (0 if >=8bpp) */
    BYTE        bReserved;       /* Reserved ( must be 0)		*/
    WORD        wPlanes;         /* Color Planes			*/
    WORD        wBitCount;       /* Bits per pixel			*/
    DWORD       dwBytesInRes;    /* How many bytes in this resource?	*/
    DWORD       dwImageOffset;   /* Where in the file is this image?	*/
} icoICONDIRENTRY, FAR * LPicoICONDIRENTRY;

typedef struct
{
    WORD            idReserved;   /* Reserved (must be 0) */
    WORD            idType;       /* Resource Type (RES_ICON or RES_CURSOR) */
    WORD            idCount;      /* How many images */
    icoICONDIRENTRY idEntries[1]; /* An entry for each image (idCount of 'em) */
} icoICONDIR, FAR * LPicoICONDIR;

typedef struct
{
    WORD offset;
    WORD length;
    WORD flags;
    WORD id;
    WORD handle;
    WORD usage;
} NE_NAMEINFO;

typedef struct
{
    WORD  type_id;
    WORD  count;
    DWORD resloader;
} NE_TYPEINFO;

#define NE_RSCTYPE_ICON        0x8003
#define NE_RSCTYPE_GROUP_ICON  0x800e

typedef struct
{
    BYTE   bWidth;
    BYTE   bHeight;
    BYTE   bColorCount;
    BYTE   bReserved;
} ICONRESDIR;

typedef struct
{
    WORD   wWidth;
    WORD   wHeight;
} CURSORDIR;

typedef struct
{   union
    { ICONRESDIR icon;
      CURSORDIR  cursor;
    } ResInfo;
    WORD   wPlanes;
    WORD   wBitCount;
    DWORD  dwBytesInRes;
    WORD   wResId;
} CURSORICONDIRENTRY;

typedef struct
{
    WORD                idReserved;
    WORD                idType;
    WORD                idCount;
    CURSORICONDIRENTRY  idEntries[1];
} CURSORICONDIR;

typedef struct {
    BYTE bWidth;
    BYTE bHeight;
    BYTE bColorCount;
    BYTE bReserved;
    WORD xHotspot;
    WORD yHotspot;
    DWORD dwDIBSize;
    DWORD dwDIBOffset;
} CURSORICONFILEDIRENTRY;

typedef struct
{
    WORD                idReserved;
    WORD                idType;
    WORD                idCount;
    CURSORICONFILEDIRENTRY  idEntries[1];
} CURSORICONFILEDIR;

/* NtUserSetCursorIconData parameter, not compatible with Windows */
struct cursoricon_frame
{
    UINT     width;    /* frame-specific width */
    UINT     height;   /* frame-specific height */
    HBITMAP  color;    /* color bitmap */
    HBITMAP  alpha;    /* pre-multiplied alpha bitmap for 32-bpp icons */
    HBITMAP  mask;     /* mask bitmap (followed by color for 1-bpp icons) */
    POINT    hotspot;
};

struct cursoricon_desc
{
    UINT flags;
    UINT num_steps;
    UINT num_frames;
    UINT delay;
    struct cursoricon_frame *frames;
    DWORD *frame_seq;
    DWORD *frame_rates;
    HRSRC rsrc;
};

#pragma pack(pop)


typedef struct keyKEYSTRUCT {
	HKEY hParentKey;
	ATOM atomKey;
	BOOL fOpen;
	HKEY hSubKey;
	HKEY hNext;
	LPSTR lpszValue;
} KEYSTRUCT;

typedef KEYSTRUCT FAR * LPKEYSTRUCT;

#define IFK_FIND	0
#define IFK_CREATE	1

extern char szWindows[20];
extern char szPrograms[20];
extern char szProgramsValue[20];
extern char szExtensions[20];
extern char szOpen[20];

extern const char lpstrMsgWndCreated[];
extern const char lpstrMsgWndDestroyed[];
extern const char lpstrMsgShellActivate[];

extern HWND SHELL_hWnd;
extern HHOOK SHELL_hHook;
extern HOOKPROC SHELL_lpProc;

extern UINT	uMsgWndCreated;
extern UINT	uMsgWndDestroyed;
extern UINT	uMsgShellActivate;

extern HDC display_dc;

extern ATOMTABLE AtomTable;

extern BOOL fRegInitialized;

extern KEYSTRUCT RootKey;

/* REG.DAT structures */

#define DATMAGIC "SHCC3.10"

typedef struct tagDATHEADER {
	char szSignature[8];	//0x0000  8 Byte  ASCII-Text: "SHCC3.10"
	DWORD unk1;		//0x0008  DWord  ?
	DWORD unk2;		//0x000C  DWord  ? (always equal the D-Word at 0x0008) Note: May be first is header size, second is offset of navblock
	DWORD dwEntries;	//0x0010  DWord  Number of entries in the navigation-block
	DWORD offsetDataBlock;	//0x0014  DWord  Offset of the data-block
	DWORD sizeDataBlock;	//0x0018  DWord  Size of the data-block
	WORD  hashsize;		//0x00??  Word   Hash size
	WORD  freeidx;		//0x00??  Word   Free index
} DATHEADER;

typedef struct tagDATNAVIGATION {
	WORD	Next;	//0x00    Word    Next Key (same level)
	WORD	Sub;	//0x02    Word    First Sub-Key (one level deeper)
	WORD	Name;	//0x04    Word    Text-Info-Record Key-Namens
	WORD	Value;	//0x06    Word    Text-Info-Record Key-Value (default)
} DATNAVIGATION;

typedef struct tagDATTEXT {
	WORD	Index;		//0x00    Word    Entry Table Index * 2 + 1
	WORD	RefCount;	//0x02    Word    number of references to this text
	WORD	Length;		//0x04    Word    Text-length
	WORD	offset;		//0x06    Word    Offset of the text-string inside the data-block
} DATTEXT;

/*
Navigation-Info-Record

Offset  Size    Contents

The values are the locical number of the block inside the file:

	offset=blocksize*blocknumber+headersize

since 2 of this values are constant:

	offset=8*blocknumber+0x20


Text-Info-Record
================


Offset  Size    Contents


*/

BOOL InitReg();
