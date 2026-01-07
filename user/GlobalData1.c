#include "user.h"

WORD GlobalAtomTable_Selector; // Selector of Global Atom Table

WORD __based(__segname("_TEXT")) USER_HeapSel = 0;  /* USER heap selector (hinstance) */
HMODULE HModuleWin = 0;
HINSTANCE HInstanceDisplay;

int  ClBorder;          /* Frame border width */

char DebugBuffer[100];  /* Buffer for DEBUG messages */

char szNullString[1]="";
char szSysError[0x14];
char szDivZero[0x14];
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
int defaultVal;

HWND HWndFocus;
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

KBINFO KbInfo;
CURSORINFO CursorInfo;
MOUSEINFO MouseInfo;

DWORD dwMouseX=0;
DWORD dwMouseY=0;

char DISPLAY[]="DISPLAY";

char RGBKeyState[0xff];
HDC tempHDC;
PDCE PDCEFirst;
