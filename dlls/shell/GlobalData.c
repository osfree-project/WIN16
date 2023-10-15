#include "Shell.h"

char szWindows[20];
char szPrograms[20];
char szProgramsValue[20];
char szExtensions[20];
char szOpen[20];

HWND SHELL_hWnd=0;
HHOOK SHELL_hHook=0;
HOOKPROC SHELL_lpProc=NULL;

UINT	uMsgWndCreated = 0;
UINT	uMsgWndDestroyed = 0;
UINT	uMsgShellActivate = 0;

HDC display_dc;

ATOMTABLE AtomTable;

BOOL fRegInitialized = FALSE;

KEYSTRUCT RootKey;

const char lpstrMsgWndCreated[] = "OTHERWINDOWCREATED";
const char lpstrMsgWndDestroyed[] = "OTHERWINDOWDESTROYED";
const char lpstrMsgShellActivate[] = "ACTIVATESHELLWINDOW";
