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



extern WORD __based(__segname("_TEXT")) GlobalAtomTable_Selector; // Selector of Global Atom Table
extern WORD __based(__segname("_TEXT")) USER_HeapSel;  /* USER heap selector */

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
    WORD csColor;      // Цветность (обычно 1 для монохромного)
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
    DWORD FAR *frame_seq;
    DWORD FAR *frame_rates;
    HRSRC rsrc;
};

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
