#include "user.h"

/* Global variables in code segment to be accessable without DS touch */
WORD __based(__segname("_TEXT")) GlobalAtomTable_Selector; // Selector of Global Atom Table
WORD __based(__segname("_TEXT")) USER_HeapSel = 0;  /* USER heap selector (hinstance) */
HANDLE __based(__segname("_TEXT")) firstDCE = 0;
HANDLE __based(__segname("_TEXT")) hGDI;

HMODULE hModuleWin = 0;
HINSTANCE hInstanceDisplay;

HWND hwndSysModal = 0;
WORD wDragWidth = 4;
WORD wDragHeight= 3;
HWND hwndActive      = 0;  /* Currently active window */
HWND hwndPrevActive  = 0;  /* Previously active window */
HWND captureWnd = 0;

HBITMAP hbitmapClose = 0;
HBITMAP hbitmapMinimize = 0;
HBITMAP hbitmapMinimizeD = 0;
HBITMAP hbitmapMaximize = 0;
HBITMAP hbitmapMaximizeD = 0;
HBITMAP hbitmapRestore = 0;
HBITMAP hbitmapRestoreD = 0;
HBITMAP hbitmapStdCheck = 0;
HBITMAP hbitmapStdMnArrow = 0;

int  ClBorder;          /* Frame border width */

char DebugBuffer[100];  /* Buffer for DEBUG messages */

char szNullString[1]="";
char szSysError[0x14];
char szDivZero[0x28];
char szUntitled[0x14];
char szError[0x14];
char szOk[0x14];
char szCancel[0x14];
char szAbort[0x14];
char szRetry[0x14];
char szIgnore[0x14];
char szYes[0x14];
char szNo[0x14];
//char szClose[0x14]; not found in user.exe resources
char szAm[0x14];
char szPm[0x14];

int CBEntries;
int DefQueueSize;
int IDelayMenuShow;
int IDelayMenuHide;

HICON HIconHand;
HICON HIconQues;
HICON HIconBang;
HICON HIconNote;
HICON HIconWindows;
HICON HIconSample;

HCURSOR HCursSizeAll;
HCURSOR	HCursNormal;
HCURSOR	HCursIBeam;
HCURSOR	HCursUpArrow;
HCURSOR	HCursSizeNWSE;
HCURSOR	HCursSizeNESW;
HCURSOR	HCursSizeNS;
HCURSOR	HCursSizeWE;

int SysMetricsDef[] =
  {
        640,            /* SM_CXSCREEN         0 */
        480,            /* SM_CYSCREEN         1 */
        20,             /* SM_CXVSCROLL        2 */
        20,             /* SM_CYHSCROLL        3 */
        18/*25*/,	/* SM_CYCAPTION        4 */
        1,              /* SM_CXBORDER         5 */
        1,              /* SM_CYBORDER         6 */
        1 /*4*/,        /* SM_CXDLGFRAME       7 */
        1 /*4*/,        /* SM_CYDLGFRAME       8 */
        20,             /* SM_CYVTHUMB         9 */
        20,             /* SM_CXHTHUMB         10 */
        32,             /* SM_CXICON           11 */
        32,             /* SM_CYICON           12 */
        32,             /* SM_CXCURSOR         13 */
        32,             /* SM_CYCURSOR         14 */
        25,             /* SM_CYMENU           15 */
        640,            /* SM_CXFULLSCREEN     16 */
        460,            /* SM_CYFULLSCREEN     17 */
        0,              /* SM_CYKANJIWINDOW    18 */
        1,              /* SM_MOUSEPRESENT     19 */
        20,             /* SM_CYVSCROLL        20 */
        20,             /* SM_CXHSCROLL        21 */
#if DEBUG
        255,            /* SM_DEBUG            22 */
#else
        0,              /* SM_DEBUG            22 */
#endif
        0,              /* SM_SWAPBUTTON       23 */
        0,              /* SM_RESERVED1        24 */
        1,              /* SM_RESERVED2        25 */
        0,              /* SM_RESERVED3        26 */
        0,              /* SM_RESERVED4        27 */
        102,            /* SM_CXMIN            28 */
        26,             /* SM_CYMIN            29 */
        18,             /* SM_CXSIZE           30 */
        18,             /* SM_CYSIZE           31 */
        4,              /* SM_CXFRAME          32 */
        4,              /* SM_CYFRAME          33 */
        102,            /* SM_CXMINTRACK       34 */
        26,             /* SM_CYMINTRACK       35 */
#if (WINVER >= 0x030a)
	5,		/* SM_CXDOUBLECLK      36 */
	4,		/* SM_CYDOUBLECLK      37 */
	48,		/* SM_CXICONSPACING    38 */
	32,		/* SM_CYICONSPACING    39 */
	0,		/* SM_MENUDROPALIGNMENT 40 */
	0,		/* SM_PENWINDOWS       41 */
	0,		/* SM_DBCSENABLED      42 */
#endif
	43,		/* SM_CMETRICS		43 */
#if 0
	3,		/* SM_CMOUSEBUTTONS    43 */
	/* Win95 */
	FALSE,		/* SM_SECURE		44 */
	2,		/* SM_CXEDGE		45 */
	2,		/* SM_CYEDGE		46 */
	48,		/* SM_CXMINSPACING	47 */
	48,		/* SM_CYMINSPACING	48 */
	16,		/* SM_CXSMICON		49 */
	16,		/* SM_CYSMICON		50 */
	16,		/* SM_CYSMCAPTION	51 */
	16,		/* SM_CXSMSIZE		52 */
	16,		/* SM_CYSMSIZE		53 */
	25,		/* SM_CXMENUSIZE	54 */
	25,		/* SM_CYMENUSIZE	55 */
	ARW_BOTTOMLEFT|ARW_LEFT, /* SM_ARRANGE		56 */
	32,		/* SM_CXMINIMIZED	57 */
	32,		/* SM_CYMINIMIZED	58 */
	640,		/* SM_CXMAXTRACK	59 */
	480,		/* SM_CYMAXTRACK	60 */
	640,		/* SM_CXMAXIMIZED	61 */
	480,		/* SM_CYMAXIMIZED	62 */
	0,		/* SM_NETWORK		63 */
	0,		/* SM_CLEANBOOT		64 */
	4,		/* SM_CXDRAG		68 */
	4,		/* SM_CYDRAG		69 */
	/* all versions */
	FALSE,		/* SM_SHOWSOUNDS	70 */
	/* Win95 */
	16,		/* SM_CXMENUCHECK	71 */
	16,		/* SM_CYMENUCHECK	72 */
	FALSE,		/* SM_SLOWMACHINE	73 */
	FALSE,		/* SM_MIDEASTENABLED	74 */
	/* all versions */
	75,		/* SM_CMETRICS		75 */
#endif
  };

int FDragFullWindows;
int FFastAltTab;
int CXYGranularity;
int CXSize;
int CYSize;
int defaultVal;

HWND hwndFocus;
HWND HWndDesktop;
HWND HWndSwitch;
HWND HWndRealPopup;

HMENU HSysMenu;

HGLOBAL MenuBase;
HGLOBAL HMenuHeap;
HGLOBAL MenuStringBase;
HGLOBAL HMenuStringHeap;


FARPROC LpSaveBitmap;
FARPROC LpDisplayCriticalSection;

HLOCAL hTaskManName;
LPSTR PTaskManName;

HMODULE HModSound;
FARPROC lpfnSoundEnable;

KBINFO KbInfo;
CURSORINFO CursorInfo;
MOUSEINFO MouseInfo;

WORD wMouseX=0;
WORD wMouseY=0;

HQUEUE hFirstQueue = 0;
HQUEUE hmemSysMsgQueue = 0;
MESSAGEQUEUE FAR *sysMsgQueue = NULL;

HCLASS firstClass = 0;

char DISPLAY[]="DISPLAY";
char SOUND[]="SOUND";

char RGBKeyState[0xff];
//PDCE PDCEFirst;

struct SysColorObjects sysColorObjects = { 0, };

