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
extern char DebugBuffer[100];
extern WORD USER_HeapSel;  /* USER heap selector */
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

extern int  ClBorder;
extern int CBEntries;
extern HMODULE HModuleWin;
extern int DefQueueSize;

extern char DISPLAY[];
extern HDC tempHDC;


#ifdef DEBUG
extern void _cdecl printf (const char *format,...);
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
             printf(__VA_ARGS__);		\
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
#define IDS_COLORS 0x01					// Colors
#define IDS_TYPEAHEAD 0x07					// TypeAhead
#define IDS_BORDER 0x0e                                         // Border
#define IDS_DEFAULTQUEUESIZE 0x0f			// DefaultQueueSize
#define IDS_SYSTEMERROR 0x4b				// System Error
#define IDS_DIVIDEBYZERO 0x4c				// Divede By Zero or Overflow Error
#define IDS_UNTITLED 0x4d					// Untitiled
#define IDS_ERROR 0x4e						// Error
#define IDS_OK 0x54							// Ok
#define IDS_CANCEL 0x55						// Cancel
#define IDS_ABORT 0x56						// Abort
#define IDS_RETRY 0x57						// Retry
#define IDS_IGNORE 0x58						// Ignore
#define IDS_YES 0x59						// Yes
#define IDS_NO 0x5a							// No
#define IDS_AM 0x5c							// am
#define IDS_PM 0x5d							// pm

#ifdef __WATCOMC__
#undef IDC_ARROW
#undef IDC_IBEAM
#undef IDC_WAIT
#undef IDC_CROSS
#undef IDC_UPARROW
#undef IDC_SIZE
#undef IDC_ICON
#undef IDC_SIZENWSE
#undef IDC_SIZENESW
#undef IDC_SIZEWE
#undef IDC_SIZENS
#undef IDC_SIZEALL
#undef IDC_NO
#undef IDC_APPSTARTING
#undef IDC_HELP
#endif

#define IDC_ARROW 32512
#define IDC_IBEAM 32513
#define IDC_WAIT 32514
#define IDC_CROSS 32515
#define IDC_UPARROW 32516
#define IDC_SIZE 32640
#define IDC_ICON 32641
#define IDC_SIZENWSE 32642
#define IDC_SIZENESW 32643
#define IDC_SIZEWE 32644
#define IDC_SIZENS 32645
#define IDC_SIZEALL 32646
#define IDC_NO 32648
#define IDC_APPSTARTING 32650
#define IDC_HELP 32651

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

KBINFO KbInfo;

typedef
struct tagCURSORINFO {
	WORD dpXRate;		//horizontal mickey/pixel ratio
	WORD dpYRate;		//vertical mickey/pixel ratio
} CURSORINFO;

CURSORINFO CursorInfo;

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

MOUSEINFO MouseInfo;

WORD WINAPI InquireMouse(MOUSEINFO FAR *MouseInfo);

WORD WINAPI InquireDisplay(CURSORINFO FAR *CursorInfo);

HANDLE WINAPI SetObjectOwner(HANDLE hObject, HANDLE hTask);

BOOL WINAPI MakeObjectPrivate(HANDLE hObject, BOOL bPrivate);

#define GetStockBrush(i) ((HBRUSH)GetStockObject(i))


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




#ifndef MK_FP
#define MK_FP(seg,off) ((void far *)(((unsigned long)(seg) << 16) | (unsigned)(off)))
#endif
