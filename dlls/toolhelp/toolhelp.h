/*
 * Copyright (C) the Wine project
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

#ifndef __WINE_TOOLHELP_H
#define __WINE_TOOLHELP_H

#include <windows.h>

#define MAX_DATA	11
#define MAX_MODULE_NAME	9
#define MAX_PATH	255
#define MAX_CLASSNAME	255

#pragma pack(push,1)

/* Global heap */

typedef struct tagGLOBALINFO
{
    DWORD dwSize;
    WORD  wcItems;
    WORD  wcItemsFree;
    WORD  wcItemsLRU;
} GLOBALINFO;
typedef GLOBALINFO FAR  *LPGLOBALINFO;

typedef struct tagGLOBALENTRY
{
    DWORD     dwSize;
    DWORD     dwAddress;
    DWORD     dwBlockSize;
    HGLOBAL   hBlock;
    WORD      wcLock;
    WORD      wcPageLock;
    WORD      wFlags;
    BOOL      wHeapPresent;
    HGLOBAL   hOwner;
    WORD      wType;
    WORD      wData;
    DWORD     dwNext;
    DWORD     dwNextAlt;
} GLOBALENTRY;
typedef GLOBALENTRY FAR *LPGLOBALENTRY;

  /* GlobalFirst()/GlobalNext() flags */
#define GLOBAL_ALL      0
#define GLOBAL_LRU      1
#define GLOBAL_FREE     2

  /* wType values */
#define GT_UNKNOWN      0
#define GT_DGROUP       1
#define GT_DATA         2
#define GT_CODE         3
#define GT_TASK         4
#define GT_RESOURCE     5
#define GT_MODULE       6
#define GT_FREE         7
#define GT_INTERNAL     8
#define GT_SENTINEL     9
#define GT_BURGERMASTER 10

/* wData values */
#define GD_USERDEFINED      0
#define GD_CURSORCOMPONENT  1
#define GD_BITMAP           2
#define GD_ICONCOMPONENT    3
#define GD_MENU             4
#define GD_DIALOG           5
#define GD_STRING           6
#define GD_FONTDIR          7
#define GD_FONT             8
#define GD_ACCELERATORS     9
#define GD_RCDATA           10
#define GD_ERRTABLE         11
#define GD_CURSOR           12
#define GD_ICON             14
#define GD_NAMETABLE        15
#define GD_MAX_RESOURCE     15

/* wFlags values */
#define GF_PDB_OWNER        0x0100      /* Low byte is KERNEL flags */

BOOL WINAPI GlobalInfo( LPGLOBALINFO lpInfo );
BOOL WINAPI GlobalFirst( LPGLOBALENTRY lpGlobal, WORD wFlags );
BOOL WINAPI GlobalNext( LPGLOBALENTRY lpGlobal, WORD wFlags) ;
BOOL WINAPI GlobalEntryHandle( LPGLOBALENTRY lpGlobal, HGLOBAL hItem );
BOOL WINAPI GlobalEntryModule( LPGLOBALENTRY lpGlobal, HMODULE hModule,
                                 WORD wSeg );

/* Local heap */

typedef struct tagLOCALINFO
{
    DWORD   dwSize;
    WORD    wcItems;
} LOCALINFO;
typedef LOCALINFO FAR   *LPLOCALINFO;

typedef struct tagLOCALENTRY
{
    DWORD   dwSize;
    HLOCAL  hHandle;
    WORD    wAddress;
    WORD    wSize;
    WORD    wFlags;
    WORD    wcLock;
    WORD    wType;
    WORD    hHeap;
    WORD    wHeapType;
    WORD    wNext;
} LOCALENTRY;
typedef LOCALENTRY FAR  *LPLOCALENTRY;

/* wHeapType values */
#define NORMAL_HEAP     0
#define USER_HEAP       1
#define GDI_HEAP        2

/* wFlags values */
#define LF_FIXED        1
#define LF_FREE         2
#define LF_MOVEABLE     4

/* wType values */
#define LT_NORMAL                   0
#define LT_FREE                     0xff
#define LT_GDI_PEN                  1   /* LT_GDI_* is for GDI's heap */
#define LT_GDI_BRUSH                2
#define LT_GDI_FONT                 3
#define LT_GDI_PALETTE              4
#define LT_GDI_BITMAP               5
#define LT_GDI_RGN                  6
#define LT_GDI_DC                   7
#define LT_GDI_DISABLED_DC          8
#define LT_GDI_METADC               9
#define LT_GDI_METAFILE             10
#define LT_GDI_MAX                  LT_GDI_METAFILE
#define LT_USER_CLASS               1   /* LT_USER_* is for USER's heap */
#define LT_USER_WND                 2
#define LT_USER_STRING              3
#define LT_USER_MENU                4
#define LT_USER_CLIP                5
#define LT_USER_CBOX                6
#define LT_USER_PALETTE             7
#define LT_USER_ED                  8
#define LT_USER_BWL                 9
#define LT_USER_OWNERDRAW           10
#define LT_USER_SPB                 11
#define LT_USER_CHECKPOINT          12
#define LT_USER_DCE                 13
#define LT_USER_MWP                 14
#define LT_USER_PROP                15
#define LT_USER_LBIV                16
#define LT_USER_MISC                17
#define LT_USER_ATOMS               18
#define LT_USER_LOCKINPUTSTATE      19
#define LT_USER_HOOKLIST            20
#define LT_USER_USERSEEUSERDOALLOC  21
#define LT_USER_HOTKEYLIST          22
#define LT_USER_POPUPMENU           23
#define LT_USER_HANDLETABLE         32
#define LT_USER_MAX                 LT_USER_HANDLETABLE

BOOL WINAPI LocalInfo( LPLOCALINFO lpLocalInfo, HGLOBAL handle );
BOOL WINAPI LocalFirst( LPLOCALENTRY lpLocalEntry, HGLOBAL handle );
BOOL WINAPI LocalNext( LPLOCALENTRY lpLocalEntry );

/* Local 32-bit heap */

typedef struct
{
    DWORD dwSize;                /* 00 */
    DWORD dwMemReserved;         /* 04 */
    DWORD dwMemCommitted;        /* 08 */
    DWORD dwTotalFree;           /* 0C */
    DWORD dwLargestFreeBlock;    /* 10 */
    DWORD dwcFreeHandles;        /* 14 */
} LOCAL32INFO;

typedef struct
{
    DWORD dwSize;                /* 00 */
    WORD hHandle;                /* 04 */
    DWORD dwAddress;             /* 06 */
    DWORD dwSizeBlock;           /* 0A */
    WORD wFlags;                 /* 0E */
    WORD wType;                  /* 10 */
    WORD hHeap;                  /* 12 */
    WORD wHeapType;              /* 14 */
    DWORD dwNext;                /* 16 */
    DWORD dwNextAlt;             /* 1A */
} LOCAL32ENTRY;

/* LOCAL32ENTRY.wHeapType flags same as LOCALENTRY.wHeapType flags */
/* LOCAL32ENTRY.wFlags same as LOCALENTRY.wFlags */
/* LOCAL32ENTRY.wType same as LOCALENTRY.wType */

BOOL WINAPI Local32Info( LOCAL32INFO *pLocal32Info, HGLOBAL handle );
BOOL WINAPI Local32First( LOCAL32ENTRY *pLocal32Entry, HGLOBAL handle );
BOOL WINAPI Local32Next( LOCAL32ENTRY *pLocal32Entry );


/* modules */

typedef struct tagMODULEENTRY
{
    DWORD      dwSize;
    char       szModule[MAX_MODULE_NAME + 1];
    HMODULE  hModule;
    WORD       wcUsage;
    char       szExePath[MAX_PATH + 1];
    HANDLE   wNext;
} MODULEENTRY;
typedef MODULEENTRY FAR *LPMODULEENTRY;

BOOL WINAPI ModuleFirst(LPMODULEENTRY lpModule);
BOOL WINAPI ModuleNext(LPMODULEENTRY lpModule);
BOOL WINAPI ModuleFindName(LPMODULEENTRY lpModule, LPCSTR lpstrName);
BOOL WINAPI ModuleFindHandle(LPMODULEENTRY lpModule, HMODULE hModule);

/* tasks */

typedef struct tagTASKENTRY
{
    DWORD        dwSize;
    HTASK        hTask;
    HTASK        hTaskParent;
    HINSTANCE    hInst;
    HMODULE      hModule;
    WORD         wSS;
    WORD         wSP;
    WORD         wStackTop;
    WORD         wStackMinimum;
    WORD         wStackBottom;
    WORD         wcEvents;
    HGLOBAL      hQueue;
    char         szModule[MAX_MODULE_NAME + 1];
    WORD         wPSPOffset;
    HANDLE       hNext;
} TASKENTRY;
typedef TASKENTRY FAR   *LPTASKENTRY;

BOOL WINAPI TaskFirst(LPTASKENTRY lpTask);
BOOL WINAPI TaskNext(LPTASKENTRY lpTask);
BOOL WINAPI TaskFindHandle(LPTASKENTRY lpTask, HTASK hTask);
DWORD  WINAPI TaskSetCSIP(HTASK hTask, WORD wCS, WORD wIP);
DWORD  WINAPI TaskGetCSIP(HTASK hTask);
BOOL WINAPI TaskSwitch(HTASK hTask, DWORD dwNewCSIP);

/* flag for TerminateApp() */
#define NO_UAE_BOX     1

/* mem info */

typedef struct tagMEMMANINFO {
	DWORD dwSize;
	DWORD dwLargestFreeBlock;
	DWORD dwMaxPagesAvailable;
	DWORD dwMaxPagesLockable;
	DWORD dwTotalLinearSpace;
	DWORD dwTotalUnlockedPages;
	DWORD dwFreePages;
	DWORD dwTotalPages;
	DWORD dwFreeLinearSpace;
	DWORD dwSwapFilePages;
	WORD wPageSize;
} MEMMANINFO;
typedef MEMMANINFO FAR *LPMEMMANINFO;

typedef struct tagSYSHEAPINFO
{
    DWORD     dwSize;
    WORD      wUserFreePercent;
    WORD      wGDIFreePercent;
    HGLOBAL   hUserSegment;
    HGLOBAL   hGDISegment;
} SYSHEAPINFO;
typedef SYSHEAPINFO FAR *LPSYSHEAPINFO;

BOOL WINAPI MemManInfo(LPMEMMANINFO lpEnhMode);
BOOL WINAPI SystemHeapInfo( LPSYSHEAPINFO pHeapInfo );

/* timer info */

typedef struct tagTIMERINFO {
	DWORD dwSize;
	DWORD dwmsSinceStart;
	DWORD dwmsThisVM;
} TIMERINFO;
typedef TIMERINFO FAR   *LPTIMERINFO;

BOOL WINAPI TimerCount( LPTIMERINFO pTimerInfo );

/* Window classes */

typedef struct tagCLASSENTRY
{
    DWORD     dwSize;
    HMODULE   hInst;              /* This is really an hModule */
    char      szClassName[MAX_CLASSNAME + 1];
    HANDLE    wNext;
} CLASSENTRY;
typedef CLASSENTRY FAR  *LPCLASSENTRY;

BOOL WINAPI ClassFirst( LPCLASSENTRY lpClassEntry );
BOOL WINAPI ClassNext( LPCLASSENTRY lpClassEntry );


/* Memory read/write */

DWORD WINAPI MemoryRead( WORD sel, DWORD offset, void FAR *buffer, DWORD count );
DWORD WINAPI MemoryWrite( WORD sel, DWORD offset, void FAR *buffer, DWORD count );

/* flags to NotifyRegister() */
#define NF_NORMAL	0	/* everything except taskswitches, debugerrors,
				 * debugstrings
				 */
#define NF_TASKSWITCH	1	/* get taskswitch information */
#define NF_RIP		2	/* get debugerrors of system */

BOOL WINAPI NotifyRegister(HTASK htask,FARPROC lpfnCallback,WORD wFlags);

#define NFY_UNKNOWN	0
#define NFY_LOADSEG	1
/* DATA is a pointer to following struct: */
typedef struct {
	DWORD	dwSize;
	WORD	wSelector;
	WORD	wSegNum;
	WORD	wType;		/* bit 0 set if this is a code segment */
	WORD	wcInstance;	/* only valid for data segment */
} NFYLOADSEG;
/* called when freeing a segment. LOWORD(dwData) is the freed selector */
#define NFY_FREESEG	2

/* called when loading/starting a DLL */
#define NFY_STARTDLL	3
typedef struct {
    DWORD      dwSize;
    HMODULE    hModule;
    WORD       wCS;
    WORD       wIP;
} NFYSTARTDLL;

/* called when starting a task. dwData is CS:IP */
#define NFY_STARTTASK	4

/* called when a task terminates. dwData is the return code */
#define NFY_EXITTASK	5

/* called when module is removed. LOWORD(dwData) is the handle */
#define NFY_DELMODULE	6

/* RIP? debugevent */
#define NFY_RIP		7
typedef struct {
	DWORD	dwSize;
	WORD	wIP;
	WORD	wCS;
	WORD	wSS;
	WORD	wBP;
	WORD	wExitCode;
} NFYRIP;

/* called before (after?) switching to a task
 * no data, callback should call GetCurrentTask
 */
#define	NFY_TASKIN	8

/* called before(after?) switching from a task
 * no data, callback should call GetCurrentTask
*/
#define NFY_TASKOUT	9

/* returns ASCII input value, dwData not set */
#define NFY_INCHAR	10

/* output debugstring (pointed to by dwData) */
#define NFY_OUTSTRING	11

/* log errors */
#define NFY_LOGERROR	12
typedef struct {
	DWORD	dwSize;
	UINT	wErrCode;
	VOID   *lpInfo; /* depends on wErrCode */
} NFYLOGERROR;

/* called for parameter errors? */
#define NFY_LOGPARAMERROR	13
typedef struct {
    DWORD       dwSize;
    UINT        wErrCode;
    FARPROC     lpfnErrorAddr;
    void      **lpBadParam;
} NFYLOGPARAMERROR;

typedef struct tagSTACKTRACEENTRY {
    DWORD dwSize;
    HTASK hTask;
    WORD wSS;
    WORD wBP;
    WORD wCS;
    WORD wIP;
    HMODULE hModule;
    WORD wSegment;
    WORD wFlags;
} STACKTRACEENTRY;
typedef STACKTRACEENTRY FAR *LPSTACKTRACEENTRY;

BOOL WINAPI StackTraceCSIPFirst(LPSTACKTRACEENTRY ste, WORD wSS, WORD wCS, WORD wIP, WORD wBP);
BOOL WINAPI StackTraceFirst(LPSTACKTRACEENTRY ste, HTASK Task);
BOOL WINAPI StackTraceNext(LPSTACKTRACEENTRY ste);

#define HQUEUE HANDLE

/* Process database (i.e. a normal DOS PSP) */
typedef struct
{
    WORD      int20;            /* 00 int 20h instruction */
    WORD      nextParagraph;    /* 02 Segment of next paragraph */
    BYTE      reserved1;
    BYTE      dispatcher[5];    /* 05 Long call to DOS */
    FARPROC	savedint22;       /* 0a Saved int 22h handler */
    FARPROC savedint23;       /* 0e Saved int 23h handler */
    FARPROC savedint24;       /* 12 Saved int 24h handler */
    WORD      parentPSP;        /* 16 Selector of parent PSP */
    BYTE      fileHandles[20];  /* 18 Open file handles */
    HANDLE  environment;      /* 2c Selector of environment */
    DWORD     saveStack;        /* 2e SS:SP on last int21 call */
    WORD      nbFiles;          /* 32 Number of file handles */
    DWORD    fileHandlesPtr;   /* 34 Pointer to file handle table */
    HANDLE	hFileHandles;     /* 38 Handle to fileHandlesPtr */
    WORD      reserved3[17];
    BYTE      fcb1[16];         /* 5c First FCB */
    BYTE      fcb2[20];         /* 6c Second FCB */
    BYTE      cmdLine[128];     /* 80 Command-line (first byte is len)*/
    BYTE      padding[16];      /* Some apps access beyond the end of the cmd line */
} PDB;

/* Task database. See 'Windows Internals' p. 226.
 * Note that 16-bit OLE 2 libs like to read it directly
 * so we have to keep entry offsets as they are.
 */
typedef struct _TDB
{
    HTASK     hNext;              /* 00 Selector of next TDB */
    DWORD     ss_sp;              /* 02 Stack pointer of task */
    WORD      nEvents;            /* 06 Events for this task */
    int       priority;           /* 08 Task priority, -32..15 */
    WORD      unused1;            /* 0a */
    HTASK     hSelf;              /* 0c Selector of this TDB */
    HANDLE    hPrevInstance;      /* 0e Previous instance of module */
    DWORD     unused2;            /* 10 */
    WORD      ctrlword8087;       /* 14 80x87 control word */
    WORD      flags;              /* 16 Task flags */
    UINT      error_mode;         /* 18 Error mode (see SetErrorMode)*/
    WORD      version;            /* 1a Expected Windows version */
    HANDLE    hInstance;          /* 1c Instance handle for task */
    HMODULE   hModule;            /* 1e Module handle */
    HQUEUE    hQueue;             /* 20 Selector of task queue */
    HTASK     hParent;            /* 22 Selector of TDB of parent */
    WORD      signal_flags;       /* 24 Flags for signal handler */
    FARPROC   sighandler;         /* 26 Signal handler */
    FARPROC   userhandler;        /* 2a USER signal handler */
    FARPROC   discardhandler;     /* 2e Handler for GlobalNotify() */
    FARPROC   int0;               /* 32 int 0 (divide by 0) handler */
    FARPROC   int2;               /* 36 int 2 (NMI) handler */
    FARPROC   int4;               /* 3a int 4 (INTO) handler */
    FARPROC   int6;               /* 3e int 6 (invalid opc) handler */
    FARPROC   int7;               /* 42 int 7 (coprocessor) handler */
    FARPROC   int3e;              /* 46 int 3e (80x87 emu) handler */
    FARPROC   int75;              /* 4a int 75 (80x87 error) handler */
    DWORD     compat_flags;       /* 4e Compatibility flags */
    BYTE      unused4[2];         /* 52 */
    DWORD unk;//struct _TEB *teb;             /* 54 Pointer to thread database */
    BYTE      unused5[8];         /* 58 */
    HANDLE    hPDB;               /* 60 Selector of PDB (i.e. PSP) */
    DWORD    dta;                /* 62 Current DTA */
    BYTE      curdrive;           /* 66 Current drive */
    char      curdir[65];         /* 67 Current directory */
    WORD      nCmdShow;           /* a8 cmdShow parameter to WinMain */
    HTASK     hYieldTo;           /* aa Next task to schedule */
    DWORD     dlls_to_init;       /* ac Ptr to DLLs to initialize */
    HANDLE    hCSAlias;           /* b0 Code segment for this TDB */
    WORD      thunks[8*4];        /* b2 Make proc instance thunks */
    char      module_name[8];     /* f2 Module name for task */
    WORD      magic;              /* fa TDB signature */
    HANDLE    hEvent;             /* fc scheduler event handle */ //?? @todo seems not in original TDB
    WORD padding;
    PDB       pdb;                /* 100 PDB for this task */
} TDB;

HMODULE WINAPI GetExePtr( HANDLE handle );

/* This function returns current DS value */
extern  unsigned short          GetDS( void );
#pragma aux GetDS               = \
        "mov    ax,ds"          \
        value                   [ax];

/* This function sets current DS value */
extern  void          SetDS( unsigned short );
#pragma aux SetDS               = \
        "mov    ds,ax"          \
        parm                   [ax];

#pragma pack(pop)

WORD WINAPI LocalCountFree(void);
WORD WINAPI LocalHeapSize(void);

#endif /* __WINE_TOOLHELP_H */
