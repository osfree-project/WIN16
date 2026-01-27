#include "user.h"

/* Global variables in code segment to be accessable without DS touch */
WORD __based(__segname("_TEXT")) GlobalAtomTable_Selector; // Selector of Global Atom Table
WORD __based(__segname("_TEXT")) USER_HeapSel = 0;  /* USER heap selector (hinstance) */
HANDLE __based(__segname("_TEXT")) firstDCE = 0;
HANDLE __based(__segname("_TEXT")) hGDI;

HMODULE HModuleWin = 0;
HINSTANCE HInstanceDisplay;

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

int FDragFullWindows;
int FFastAltTab;
int CXYGranularity;
int CXScreen;
int CYScreen;
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

LONG iMouseX=0;
LONG iMouseY=0;

HQUEUE hFirstQueue = 0;
HQUEUE hmemSysMsgQueue = 0;
HCLASS firstClass = 0;

char DISPLAY[]="DISPLAY";

char RGBKeyState[0xff];
//PDCE PDCEFirst;


