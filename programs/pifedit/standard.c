#include "main.h"

/****************************************************************************
 *
 *  FUNCTION :	StandardMode(HWND)
 *
 *  PURPOSE  :	Puts up a dialog box for Standard mode
 *
 *  ENTRY    :	HWND	hWnd;		// Parent window handle
 *
 *  RETURNS  :	VOID
 *
 ****************************************************************************/

VOID StandardMode(HWND hWnd)
{
    FARPROC	lpfnSMMsgProc;

    lpfnSMMsgProc = MakeProcInstance((FARPROC) SMMsgProc, Globals.hInst);

    DialogBox(Globals.hInst, MAKEINTRESOURCE(IDD_STANDARD), hWnd, (DLGPROC) lpfnSMMsgProc);

    FreeProcInstance(lpfnSMMsgProc);
}

/****************************************************************************
 *
 *  FUNCTION :	StandardModeBitchBox(HWND)
 *
 *  PURPOSE  :	Puts up a dialog box to warn about Standard mode
 *
 *  ENTRY    :	HWND	hWnd;		// Parent window handle
 *
 *  RETURNS  :	VOID
 *
 ****************************************************************************/

VOID StandardModeBitchBox(HWND hWnd)
{
    FARPROC	lpfnSMMsgProc;

    MessageBeep(MB_ICONEXCLAMATION);

    lpfnSMMsgProc = MakeProcInstance((FARPROC) SMMsgProc, Globals.hInst);

    DialogBox(Globals.hInst, MAKEINTRESOURCE(IDD_STANDARDBITCH), hWnd, (DLGPROC) lpfnSMMsgProc);

    FreeProcInstance(lpfnSMMsgProc);
}


/****************************************************************************
 *
 *  FUNCTION :	SMMsgProc(HWND, UINT, WPARAM, LPARAM)
 *
 *  PURPOSE  :	Message proc for the Standard mode bitch dialog box
 *
 *  ENTRY    :	HWND	hWnd;		// Window handle
 *		UINT	msg;		// WM_xxx message
 *		WPARAM	wParam; 	// Message 16-bit parameter
 *		LPARAM	lParam; 	// Message 32-bit parameter
 *
 *  RETURNS  :	FALSE	- Message has been processed
 *		TRUE	- DefDlgProc() processing required
 *
 ****************************************************************************/

BOOL CALLBACK _export SMMsgProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    switch (msg) {
	case WM_COMMAND:
	    switch (wParam) {	// id
		case IDOK:
		case IDCANCEL:
		    EndDialog(hWnd, TRUE);
	    }
	    return (0);

	case WM_INITDIALOG:
	    return (TRUE);
    }

    return (FALSE);
}

