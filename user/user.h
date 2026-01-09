#define OEMRESOURCE
#include <windows.h>

// Message Queue structures. See Matt Pietrek for description.

typedef struct tagINTERNALMSG{
	DWORD ExtraInfo;
	MSG msg;
} INTERNALMSG, * PINTERNALMSG, FAR * LPINTERNALMSG;

typedef struct tagQUEUE{
	WORD NextQueue;
	HANDLE OwningTask;
	WORD MessageSize;
	WORD NumMessages;
	WORD ReadPtr;
	WORD WritePtr;
	WORD Size;
	LONG MessageTime;
	POINT MessagePoint;
	WORD reserved;
	DWORD ExtraInfo;
	WORD reserved2;
	LONG SendMessageLParam;
	WORD SendMessageWParam;
	WORD SendMessageMessage;
	HWND SendMessageHWnd;
	WORD QuitFlag;
	int ExitCode;
	WORD flags;
	DWORD reserved3;
	WORD ExpWinVersion;
	WORD SendingHQ;
	WORD sendmsg_helper1;
	WORD sendmsg_helper2;
	WORD PaintCount;
	WORD TimersCount;
	WORD ChangeBits;
	WORD WakeBits;
	WORD WakeMask;
	WORD SendMessageResult1;
	WORD SendMessageResult2;
	WORD Hook;
	BYTE Hooks2[30];
	INTERNALMSG MessageArrayStart[1];
} QUEUE;
typedef QUEUE * PQUEUE;
typedef QUEUE NEAR * NPQUEUE;
typedef QUEUE FAR * LPQUEUE;

#define HQUEUE HGLOBAL

HQUEUE WINAPI SetTaskQueue(HTASK, HQUEUE);

typedef struct tagWNDCLASSEX {
	UINT	cbSize;
	UINT	style;
	WNDPROC	lpfnWndProc;
	int	cbClsExtra;
	int	cbWndExtra;
	HANDLE	hInstance;
	HICON	hIcon;
	HCURSOR	hCursor;
	HBRUSH	hbrBackground;
	LPCSTR	lpszMenuName;
	LPCSTR	lpszClassName;
	HICON	hIconSm;
} WNDCLASSEX;

typedef WNDCLASSEX	*PWNDCLASSEX;
typedef WNDCLASSEX NEAR	*NPWNDCLASSEX;
typedef WNDCLASSEX FAR	*LPWNDCLASSEX;

typedef struct tagINTWNDCLASS {
                HANDLE hcNext;
                WORD wSig;
                ATOM atomCls;
                HANDLE hDCE;
                WORD cClsWnds;
                WNDCLASS wndClass;
                WORD wData[1];
} INTWNDCLASS;

/* Private class style bit */
#define	CS_SYSTEMGLOBAL	0x8000

#define	SYSLOCAL	0
#define	SYSGLOBAL	1
#define	APPLOCAL	2
#define	APPGLOBAL	3

/*
struct CLASS{
                HANDLE hNext;
                WORD wSign;
                ATOM atomClass;
                HANDLE hDCE;
                WORD wNumWin;
                WNDCLASS wndClass;
                WORD wData[1];
};
Поля:
hNext   Адрес следующей структуры CLASS в сегменте данных USER.
wSign   Сигнатура 'NK'.
atomCls Атом в сегменте данных USER, содержащий имя класса.
hDCE    Если для окон класса установлено CS_CLASSDC, то это дескриптор соответствующей структуры DCE, иначе NULL.
wNumWin Текущее количество окон этого класса.
wndClass        Копия соответствующей структуры WNDCLASS с обнуленными полями lpszMenuName и lpszClassName.
wData   Дополнительные данные экземпляра окна ???.

typedef struct tagWNDCLASS {
    UINT        style;
    WNDPROC     lpfnWndProc;
    int         cbClsExtra;
    int         cbWndExtra;
    HINSTANCE   hInstance;
    HICON       hIcon;
    HCURSOR     hCursor;
    HBRUSH      hbrBackground;
    LPCSTR      lpszMenuName;
    LPCSTR      lpszClassName;
} WNDCLASS;
typedef WNDCLASS        *PWNDCLASS;
typedef WNDCLASS NEAR   *NPWNDCLASS;
typedef WNDCLASS FAR    *LPWNDCLASS;

*/


typedef	struct	tagCLASSINFO {
    struct tagCLASSINFO	far * lpClassNext;	/* ptr to next class */
    struct tagCLASSINFO	far * lpClassPrev;	/* ptr to prev class */
    WORD wSign;	/* 'NK' */
    ATOM	atmClassName;		/* class name atom */
    HDC		hDC;			/* class DC handle */
    int		nUseCount;		/* Usage count  - private */
    WNDCLASS	wndClass;

    UINT	wClassType;		/* class type */

//    UINT	style;			/* class style */
//    WNDPROC	lpfnWndProc;		/* class procedure */
// This part is non-standard and seems to be translation  Native <> Binary layer
//    WNDPROC	lpfnBinToNat;		/* BINTONAT conversion procedure */
//    WNDPROC	lpfnNatToBin;		/* NATTOBIN conversion procedure */
//    int		cbClsExtra;		/* class extra bytes */
//    int		cbWndExtra;		/* window extra bytes */
//    HANDLE	hModule;		/* class module instance handle */
//    HICON	hIcon;			/* class icon resource handle */
//    HCURSOR	hCursor;		/* class cursor resource handle */
//    HBRUSH	hbrBackground;		/* class backgr. brush handle */
//    LPSTR	lpMenuName;		/* menu name string */

    LPSTR	lpClsExtra;		/* ptr to class extra bytes */
// No such in Win 3.x
//    HICON	hIconSm;		/* (WIN32) small class icon */
} CLASSINFO;
typedef CLASSINFO	*PCLASSINFO;
typedef CLASSINFO NEAR	*NPCLASSINFO;
typedef CLASSINFO FAR	*LPCLASSINFO;

/* Global Data */

extern char szNullString[1];
extern char DebugBuffer[100];
extern char szSysError[0x14];
extern char szDivZero[0x14];
extern char szUntitled[0x14];
extern char szError[0x14];
extern char szOk[0x14];
extern char szCancel[0x14];
extern char szAbort[0x14];
extern char szRetry[0x14];
extern char szIgnore[0x14];
extern char szYes[0x14];
extern char szNo[0x14];
//char szClose[0x14]; not found in user.exe resources
extern char szAm[0x14];
extern char szPm[0x14];

#ifdef DEBUG
extern void _cdecl printf (char far *format,...);
#define OutputDebugString printf
#define FUNCTION_START \
{\
	OutputDebugString(__FUNCTION__ " start\r\n");\
}
#define FUNCTION_END \
{\
	OutputDebugString(__FUNCTION__ " end\r\n");\
}


//wsprintf(DebugBuffer, __VA_ARGS__); \	OutputDebugString(DebugBuffer); \		OutputDebugString("\r\n"); 

#define TRACE(...) \
	{ \
             printf(__VA_ARGS__); printf("\r\n"); \
	}

#define WARN(...) \
	{ \
		OutputDebugString("WARNING: "); \
		TRACE(__VA_ARGS__); \
	};

#define FIXME(...) \
	{ \
		OutputDebugString("FIXME: "); \
		TRACE(__VA_ARGS__); \
	};
#else
#define FUNCTION_START
#define FUNCTION_END
#define TRACE
#define WARN
#define FIXME
#endif

/* Resources */

#define IDS_WINDOWS 0x00					// Windows
#define IDS_COLORS 0x01						// Colors
#define IDS_PATTERN 0x2						// Patterm
#define IDS_FONTS 0x3						// Fonts
#define IDS_CURSORBLINKRATE 0x4					// CursorBlinkRate
#define IDS_SWAPMOUSEBUTTONS 0x5				// SwapMouseButtons
#define IDS_DOUBLECLICKSPEED 0x6				// DoubleClickSpeed
#define IDS_TYPEAHEAD 0x07					// TypeAhead
#define IDS_GRIDGRANULARITY 0x08				// GridGranularity
#define IDS_BEEP 0x09						// Beep
#define IDS_BORDER 0x0e                                         // Border
#define IDS_DEFAULTQUEUESIZE 0x0f				// DefaultQueueSize
#define IDS_SYSTEMERROR 0x4b					// System Error
#define IDS_DIVIDEBYZERO 0x4c					// Divide By Zero or Overflow Error
#define IDS_UNTITLED 0x4d					// Untitiled
#define IDS_ERROR 0x4e						// Error
#define IDS_DESKTOP 0x50					// Desktop
#define IDS_OK 0x54						// Ok
#define IDS_CANCEL 0x55						// Cancel
#define IDS_ABORT 0x56						// Abort
#define IDS_RETRY 0x57						// Retry
#define IDS_IGNORE 0x58						// Ignore
#define IDS_YES 0x59						// Yes
#define IDS_NO 0x5a						// No
#define IDS_AM 0x5c						// am
#define IDS_PM 0x5d						// pm
#define IDS_MENUSHOWDELAY 0x5e					// MenuShowDelay
#define IDS_MENUHIDEDELAY 0x5f					// MenuHideDelay
#define IDS_MENUDROPALIGNMENT 0x60				// MenuDropAlignment
#define IDS_DOUBLECLICKWIDTH 0x61				// DoubleClickWidth
#define IDS_DOUBLECLICKHEIGHT 0x62				// DoubleClickHeight
#define IDS_DRAGFULLWINDOWS 0x6b				// DragFullWindows
#define IDS_COOLSWITCH 0x6f					// CoolSwitch

#define IDM_SYSMENU 0x01


/* Varoius undocumented protos */
HANDLE WINAPI FarGetOwner( HGLOBAL handle );
int WINAPI SetSpeed(int rate);
VOID WINAPI Resurrection(HDC hdc, WORD w1, WORD w2, WORD w3, WORD w4, WORD w5, WORD w6);
WORD WINAPI LocalCountFree();
WORD WINAPI LocalHeapSize();

#define GlobalPtrHandle(lp) \
  ((HGLOBAL)LOWORD(GlobalHandle(SELECTOROF(lp))))

#define     GlobalUnlockPtr(lp)      \
                GlobalUnlock(GlobalPtrHandle(lp))

#define GlobalFreePtr(lp) \
  (GlobalUnlockPtr(lp),(BOOL)GlobalFree(GlobalPtrHandle(lp)))

#define GlobalAllocPtr(flags, cb) \
  (GlobalLock(GlobalAlloc((flags), (cb))))


WORD WINAPI GetExeVersion(void);


/* Extra (undocumented) queue wake bits - see "Undoc. Windows" */
#define QS_SMRESULT      0x8000  /* Queue has a SendMessage() result */
#define QS_SMPARAMSFREE  0x4000  /* SendMessage() parameters are available */

typedef
struct tagKBINFO {
  char kbRanges[4]; //Far East ranges for KANJI
  WORD kbStateSize; //#bytes of state info maintained by TOASCII
} KBINFO;

typedef
struct tagCURSORINFO {
	WORD dpXRate;		//horizontal mickey/pixel ratio
	WORD dpYRate;		//vertical mickey/pixel ratio
} CURSORINFO;


WORD WINAPI InquireKeyboard(KBINFO FAR *KbInfo);

typedef
struct tagMOUSEINFO {
	BYTE msExists;	// true => mouse exists
	BYTE msRelative;  // true => relative coordinate
	WORD msNumButtons; // number of buttons on the mouse
	WORD msRate; // maximum rate of mouse input events
	WORD msXThresh; // threshold before acceleration
	WORD msYThresh;
	WORD msXRes;// x resolution
	WORD msYRes;// y resolution
} MOUSEINFO;



/* Global variables in code segment to be accessable without DS touch */
extern WORD __based(__segname("_TEXT")) GlobalAtomTable_Selector; // Selector of Global Atom Table
extern WORD __based(__segname("_TEXT")) USER_HeapSel;  /* USER heap selector (hinstance) */
extern HANDLE __based(__segname("_TEXT")) firstDCE;

extern HMODULE HModuleWin;
extern HINSTANCE HInstanceDisplay;

extern int ClBorder;
extern int CBEntries;
extern int IDelayMenuShow;
extern int IDelayMenuHide;
extern int DefQueueSize;

extern char DISPLAY[];
extern HDC tempHDC;

//extern PDCE PDCEFirst;
HDC WINAPI GetDCState(HDC);
void WINAPI SetDCState(HDC, HDC);

extern KBINFO KbInfo;
extern CURSORINFO CursorInfo;
extern MOUSEINFO MouseInfo;

extern HICON HIconWindows;
extern HICON HIconSample;
extern HICON HIconHand;
extern HICON HIconQues;
extern HICON HIconBang;
extern HICON HIconNote;

extern HCURSOR HCursSizeAll;
extern HCURSOR HCursNormal;
extern HCURSOR HCursIBeam;
extern HCURSOR HCursUpArrow;
extern HCURSOR HCursSizeNWSE;
extern HCURSOR HCursSizeNESW;
extern HCURSOR HCursSizeNS;
extern HCURSOR HCursSizeWE;

extern int FDragFullWindows;
extern int FFastAltTab;
extern int CXYGranularity;
extern int CXScreen;
extern int CYScreen;
extern int CXSize;
extern int CYSize;
extern int defaultVal;

extern HWND HWndFocus;
extern HWND HWndDesktop;
extern HWND HWndSwitch;
extern HWND HWndRealPopup;

extern HMENU HSysMenu;

extern HGLOBAL MenuBase;
extern HGLOBAL HMenuHeap;
extern HGLOBAL MenuStringBase;
extern HGLOBAL HMenuStringHeap;

extern FARPROC LpSaveBitmap;
extern FARPROC LpDisplayCriticalSection;

extern HLOCAL hTaskManName;
extern LPSTR PTaskManName;

WORD WINAPI InquireMouse(MOUSEINFO FAR *MouseInfo);
VOID WINAPI mouse_event(VOID);
VOID WINAPI MouseEnable(FARPROC proc);

WORD WINAPI InquireDisplay(CURSORINFO FAR *CursorInfo);

VOID WINAPI keybd_event(VOID);
extern char RGBKeyState[0xff];

VOID WINAPI KeyboardEnable(FARPROC proc, LPBYTE lpKeyState);

HANDLE WINAPI SetObjectOwner(HANDLE hObject, HANDLE hTask);

BOOL WINAPI MakeObjectPrivate(HANDLE hObject, BOOL bPrivate);

#define GetStockBrush(i) ((HBRUSH)GetStockObject(i))

VOID WINAPI GlobalInitAtom(void);

void far * _fmemcpy(void far * s1, void const far * s2, unsigned length);
void far * _fmemset (void far *start, int c, unsigned int len);
int _islower(int c);
int _isupper(int c);
int toupper(int c);
int _tolower(int c);
int isalnum(int c);
int isalpha(int c);
int _fstrnicmp(char const far *s1, const char far *s2, unsigned int n);
char far *itoa(int i);
char far *uitoa(unsigned int i);
char far *itox(int m);


#define  UserLocalAlloc(tag, flags, size)   LocalAlloc(flags, size)

#ifndef MK_FP
#define MK_FP(seg,off) ((void far *)(((unsigned long)(seg) << 16) | (unsigned)(off)))
#endif

VOID WINAPI EnableSystemTimers();

extern  unsigned short          GetDS( void );
#pragma aux GetDS               = \
        "mov    ax,ds"          \
        value                   [ax];


extern  void PushDS( void );
#pragma aux PushDS = "push ds";

extern  void PopDS( void );
#pragma aux PopDS = "pop ds";

/* This function sets current DS value */
extern  void          SetDS( unsigned short );
#pragma aux SetDS               = \
        "mov    ds,ax"          \
        parm                   [ax];


#define IMAGE_BITMAP   0
#define IMAGE_ICON     1
#define IMAGE_CURSOR   2
#define IMAGE_ENHMETAFILE  3

#define LR_DEFAULTCOLOR     0x0000
#define LR_MONOCHROME       0x0001
#define LR_COLOR            0x0002
#define LR_COPYRETURNORG    0x0004
#define LR_COPYDELETEORG    0x0008
#define LR_LOADFROMFILE     0x0010
#define LR_LOADTRANSPARENT  0x0020
#define LR_DEFAULTSIZE      0x0040
#define LR_LOADMAP3DCOLORS  0x1000
#define LR_CREATEDIBSECTION 0x2000
#define LR_COPYFROMRESOURCE 0x4000
#define LR_SHARED           0x8000

HMODULE WINAPI GetExePtr(HANDLE h);
VOID WINAPI FarSetOwner(HANDLE hMem, WORD wOwnerPDB);

#pragma pack(1)
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

typedef struct tagCURSORICONINFO
{
    POINT   ptHotSpot;
    WORD    nWidth;
    WORD    nHeight;
    WORD    nWidthBytes;
    BYTE    bPlanes;
    BYTE    bBitsPerPixel;
} CURSORICONINFO;

// From Norton's Writing Windows Device Drivers
//
VOID FAR PASCAL MoveCursor(WORD wAbsX, WORD wAbsY);
VOID FAR PASCAL CheckCursor(VOID);
#pragma pack(push, 1)  // Отключаем выравнивание для точного соответствия ассемблерной структуре

typedef struct tagCURSORSHAPE {
    WORD csHotX;       // Горячая точка X
    WORD csHotY;       // Горячая точка Y
    WORD csWidth;      // Ширина курсора в пикселях
    WORD csHeight;     // Высота курсора в пикселях
    WORD csWidthBytes; // Количество байт в одной строке маски
    WORD csColor;      // Цветность
    // Примечание: здесь следуют данные масок
    // Размер массива: csHeight * csWidthBytes * 2 байт
    // Первые csHeight * csWidthBytes байт - AND маска
    // Следующие csHeight * csWidthBytes байт - XOR маска
    BYTE csBits[1];    // Массив переменной длины для данных масок
} CURSORSHAPE, FAR * LPCURSORSHAPE;

#pragma pack(pop)  // Восстанавливаем выравнивание

VOID FAR PASCAL DisplaySetCursor(LPCURSORSHAPE lpCursorShape);

extern DWORD dwMouseX;
extern DWORD dwMouseY;


WORD WINAPI CreateSystemTimer(WORD wFreq, FARPROC IpCallback);

extern HMODULE HModSound;
extern FARPROC lpfnSoundEnable;
extern HQUEUE HQSysQueue;

WORD WINAPI GetTaskQueue(HANDLE hTask);

#define ME_MOVE 0x01
#define ME_LDOWN 0x02
#define ME_LUP 0x04
#define ME_RDOWN 0x08
#define ME_RUP 0x10

#pragma pack(1)

typedef struct tagGDIOBJHDR
{
    HANDLE      hNext;
    WORD        wMagic;
    DWORD       dwCount;
    WORD        wMetaList;
/* additional 3.1 debug fields from here
WORD wSelCount;
HANDLE hOwner;
*/
} GDIOBJHDR, FAR * LPGDIOBJHDR;

typedef struct tagGDIOBJDBG
{
    HANDLE      hNext;
    WORD        wMagic;
    DWORD       dwCount;
    WORD        wMetaList;
    WORD wSelCount;
    HANDLE hOwner;

} GDIOBJDBG, FAR * LPGDIOBJDBG;

typedef struct tagDC {
	GDIOBJHDR header; // 00h
	BYTE byFlags; // OAh
	BYTE byFlags2; // OBh
	HANDLE hMetaFile; // OCh
	HRGN hrgnClip; // OEh handle to (reclangular) clip region
	HANDLE hPDevice; // 10h Phys device handle
	HANDLE hLPen; // 12h Log. pen
	HANDLE hLBrush; // 14h Log. brush
	HANDLE hLFont; // 16h Log. Font
	HANDLE hBitmap; // 18h Selected bitmap
	HANDLE dchPal; // 1Ah Selected palette
	HANDLE hLDevice; // 1Ch Log. device
	HRGN hRaoClip; // 1Eh clip region
	HANDLE hPDeviceBlock; // 20h
	HANDLE hPPen; // 22h Phys. pen
	HANDLE hPBrush; // 24h Phys. brush
	HANDLE hPFontTrans; // 26h
	HANDLE hPFont; // 28h Phys. font
	LPVOID lpPDevice; // 2Ah
	WORD pLDevice; // 2Eh near pointer to log. device info
	WORD pRaoClip; // 30h near pointer to clip region
	WORD pPDeviceBlock; // 32h near pointer to GDIINFO
	WORD pPPen; // 34h
	WORD pPBrush; // 36h
	WORD pPFontTrans; // 38h near pointer to hPFontTrans
	LPVOID lpPFont; // 3Ah Font engine entrypoint
	int nPFTIndex; // 3Eh
	LPVOID Transform; // 40h
	// Begin DRAWMODE structure - see DDK doc *1
	WORD wROP2; // 44h Raster Op drawing mode
	WORD wBkMode; // 46h Background mode (opaque/transparent)
	DWORD dwBkColor; // 48h Phys. Background color
	DWORD dwTextColor; // 4Ch Phys. text color
	int nTBreakExtra; // 50h Text padding: ExtTextOut justification
	int nBreakExtra; // 52h pad per break = nTBreakExtra/BreakCount
	WORD wBreakErr; // 54h SetTextJustify called with nBreakExtra=O?
	int nBreakRem; // 56h remainder of TBreakExtra/nBreakCount
	int nBreakCount; // 58h Count of break characters in string
	int nCharExtra; // 5Ah Per char additional padding
	DWORD crLbkColor; // 5Ch Logical background color
	DWORD crLTextColor; // 60h Logical text color
	// End DRAWMODE structure *1
	int LCursPosX; // 64h
	int LCursPosY; // 66h
	int WndOrgX; // 68h
	int WndOrgY; // 6Ah
	int WndExtX; // 6Ch
	int WndExtY; // 6Eh
	int VportOrgX; // 70h
	int VportOrgY; // 72h
	int VportExtX; // 74h
	int VportExtY; // 76h
	int UserVptOrgX; // 78h
	int UserVptOrgY; // 7Ah
	WORD wMapMode; // 7Ch
	WORD wXFormFlags; // 7Eh
	WORD wRelAbs; // 80h
	WORD wPolyFillMode; //82h
	WORD wStretchBltMode; // 84h
	BYTE byPlanes; // 86h
	BYTE byBitsPix; // 87h
	WORD wPenWidth; // 88h
	WORD wPenHeight; // .8Ah
	WORD wTextAlign; // 8Ch
	DWORD dwMapperFlags; // 8Eh
	WORD wBrushOrgX; // 92h
	WORD wBrushOrgY; // 94h
	WORD wFontAspectX; // 96h
	WORD wFontAspectY; // 98h
	HANDLE hFontWeights; // 9Ah
	WORD wDCSaveLevel; // 9Ch
	WORD wcDCLocks; // 9Eh
	HRGN hVisRgn; // AOh
	WORD wDCOrgX; // A2h
	WORD wDCOrgY; // A4h
	FARPROC lpfnPrint; // A6h
	WORD wDCLogAtom; // AAh
	WORD wDCPhysAtom; // ACh
	WORD wDCFileAtom; // AEh
	WORD wPostScaleX; // BOh
	WORD wPostScaleY; // B2h
	union {
		struct { // 3.0
			WORD wB4; // B4h
			RECT rectB6; // B6h
			WORD wDCGlobFlags; // BEh
			WORD wC0; // COh
		} tail_3_0; // 3.0
		struct { // 3.1
			RECT rectBounds; // B4h
			RECT rectLVB; // BCh
			FARPROC lpfnNotify; // C4h
			LPSTR lpHookData; // C8h
			WORD wDCGlobFlags;
			HDC hDCNext;
		} tail_3_1;
	} dc_tail;
} DC, FAR *LPDC;

#define CLASS_MAGIC   0x4b4e      /* 'NK' */

#pragma pack(1)

#define HCLASS HANDLE

/* !! Don't change this structure (see GetClassLong()) */
typedef struct tagCLASS
{
    HCLASS       hNext;         /* Next class */
    WORD         wMagic;        /* Magic number (must be CLASS_MAGIC) */
    ATOM         atomName;      /* Name of the class */
    HANDLE       hdce;          /* Class DCE (if CS_CLASSDC) */
    WORD         cWindows;      /* Count of existing windows of this class */
    WNDCLASS     wc;		/* Class information */
    WORD         wExtra[1];     /* Class extra bytes */
} CLASS;

typedef struct tagWND
{
    struct tagWND *next;         /* Next sibling */
    struct tagWND *child;        /* First child */
    struct tagWND *parent;       /* Window parent (from CreateWindow) */
    struct tagWND *owner;        /* Window owner */
    DWORD        dwMagic;        /* Magic number (must be WND_MAGIC) */
    HWND         hwndSelf;       /* Handle of this window */
    HCLASS       hClass;         /* Window class */
    HANDLE       hInstance;      /* Window hInstance (from CreateWindow) */
    RECT         rectClient;     /* Client area rel. to parent client area */
    RECT         rectWindow;     /* Whole window rel. to parent client area */
    RECT         rectNormal;     /* Window rect. when in normal state */
    POINT        ptIconPos;      /* Icon position */
    POINT        ptMaxPos;       /* Maximized window position */
    HGLOBAL      hmemTaskQ;      /* Task queue global memory handle */
    HRGN         hrgnUpdate;     /* Update region */
    HWND         hwndLastActive; /* Last active popup hwnd */
    WNDPROC      lpfnWndProc;    /* Window procedure */
    DWORD        dwStyle;        /* Window style (from CreateWindow) */
    DWORD        dwExStyle;      /* Extended style (from CreateWindowEx) */
    HANDLE       hdce;           /* Window DCE (if CS_OWNDC or CS_CLASSDC) */
    HANDLE       hVScroll;       /* Vertical scroll-bar info */
    HANDLE       hHScroll;       /* Horizontal scroll-bar info */
    UINT         wIDmenu;        /* ID or hmenu (from CreateWindow) */
    HANDLE       hText;          /* Handle of window text */
    WORD         flags;          /* Misc. flags (see below) */
//    Window       window;         /* X window (only for top-level windows) */
    HMENU        hSysMenu;	 /* window's copy of System Menu */
    HANDLE       hProp;          /* Handle of Properties List */
    WORD         wExtra[1];      /* Window extra bytes */
} WND;

#pragma pack(pop)

  /* WND flags values */
#define WIN_NEEDS_BEGINPAINT   0x0001 /* WM_PAINT sent to window */
#define WIN_NEEDS_ERASEBKGND   0x0002 /* WM_ERASEBKGND must be sent to window*/
#define WIN_NEEDS_NCPAINT      0x0004 /* WM_NCPAINT must be sent to window */
#define WIN_RESTORE_MAX        0x0008 /* Maximize when restoring */
#define WIN_INTERNAL_PAINT     0x0010 /* Internal WM_PAINT message pending */
#define WIN_NO_REDRAW          0x0020 /* WM_SETREDRAW called for this window */
#define WIN_GOT_SIZEMSG        0x0040 /* WM_SIZE has been sent to the window */
#define WIN_NCACTIVATED        0x0080 /* last WM_NCACTIVATE was positive */
#define WIN_MANAGED            0x0100 /* Window managed by the X wm */

#define WIN_CLASS_INFO(wndPtr)   (CLASS_FindClassPtr((wndPtr)->hClass)->wc)
#define WIN_CLASS_STYLE(wndPtr)  (WIN_CLASS_INFO(wndPtr).style)

extern void CLASS_DumpClass( HCLASS hClass );
extern void CLASS_WalkClasses(void);
extern HCLASS CLASS_FindClassByName( LPCSTR name, HINSTANCE hinstance,
                                     CLASS **ptr );
extern CLASS * CLASS_FindClassPtr( HCLASS hclass );

extern HANDLE hGDI;

/* GDI objects magic numbers */
#define PEN_MAGIC             0x4f47
#define BRUSH_MAGIC           0x4f48
#define FONT_MAGIC            0x4f49
#define PALETTE_MAGIC         0x4f4a
#define BITMAP_MAGIC          0x4f4b
#define REGION_MAGIC          0x4f4c
#define DC_MAGIC              0x4f4d
#define DISABLED_DC_MAGIC     0x4f4e
#define META_DC_MAGIC         0x4f4f
#define METAFILE_MAGIC        0x4f50
#define METAFILE_DC_MAGIC     0x4f51
#define MAGIC_DONTCARE	      0xffff
