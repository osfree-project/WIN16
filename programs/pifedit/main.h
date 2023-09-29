#include <stdlib.h>		// For _MAX_DIR, _MAX_DRIVE, _MAX_FNAME, _MAX_EXT, _MAX_PATH, _fullpath, _splitpath, _makepath

#include <windows.h>
#include <commdlg.h>

#include <pif.h>		// .PIF structure

/* Global variables */

typedef struct
{
	char	szAppName[20];		// "PifEdit"
	char	szAppTitle[64]; 	// "PIF Editor"
	char	szUntitled[20]; 	// "(Untitled)"
	char	szFilter[80];		// "PIF Files(*.PIF)\n*.pif\n\0"
	char	szWindowTitle[128];

	HINSTANCE hInst;

	HICON	hIcon;			// Icon for minimized state

	HBRUSH	hbrHelp;
	HFONT	hFontDlg;

	HFONT	hFontHelp;

	int	nActiveDlg; 	// Active dialog int resource, or zero
	HWND	hWndDlg;
	RECT	rectDlg;		// Used to permanently remember the dialog size
	int	cxDlg, cyDlg;

	BOOL	fUntitled;	// No .PIF loaded yet

	PIF	pifModel;	// Complete 545-byte .PIF image
	PIF	pifBackup;	// Complete 545-byte .PIF image before editing

	BOOL fDefDlgEx;

	OPENFILENAME	ofn;		// For GetOpenFileName() and GetSaveFileName

	char	szDirName[256];

	char	szFile[_MAX_PATH];
	char	szExtension[5];
	char	szFileTitle[16];

	WORD	wHotkeyScancode;	// 0 indicates 'None'
	BYTE	bHotkeyBits;	//  Bits 24-31 of WM_KEYDOWN

	HLOCAL	hPIF;	// HANDLE to LocalAlloc()'ed .PIF structure

	BOOL	fCheckOnKillFocus;
	UINT	idHelpCur;

	UINT	auDlgHeights[];
} PIFEDIT_GLOBALS;

extern PIFEDIT_GLOBALS Globals;

VOID	FAR cdecl DebugPrintf(LPSTR szFormat, ...);
VOID	FAR cdecl MessageBoxPrintf(LPSTR szFormat, ...);

VOID	ControlsToPIF(HWND hWnd);
