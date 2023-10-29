#include <windows.h>
#include <penwin.h>

#define IDS_PACKAGE_NAME 101

#define IDD_SWITCHTO		IDOK
#define IDD_TASKLIST		0x64
#define IDD_ENDTASK		0x65
#define IDD_CASCADE		0x66
#define IDD_TILE		0x67
#define IDD_ARRANGEICONS	0x68

VOID WINAPI TileChildWindows(HWND,WORD);
VOID WINAPI CascadeChildWindows(HWND,WORD);
VOID WINAPI SwitchToThisWindow(HWND, BOOL);

