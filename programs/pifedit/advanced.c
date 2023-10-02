/************************  The Qualitas PIF Editor  ***************************
 *									      *
 *	     (C) Copyright 1992, 1993 Qualitas, Inc.  GNU General Public License version 3.    *
 *									      *
 *  MODULE   :	QPIFEDIT.C - Main source module for QPIFEDIT.EXE	      *
 *									      *
 *  HISTORY  :	Who	When		What				      *
 *		---	----		----				      *
 *		WRL	11 DEC 92	Original revision		      *
 *		WRL	11 MAR 93	Changes for TWT 		      *
 *		RCC	21 JUN 95	Added Ctl3d, tightened code a bit     *
 *									      *
 ******************************************************************************/

#include "main.h"

/****************************************************************************
 *
 *  FUNCTION :	AdvancedMsgProc(HWND, UINT, WPARAM, LPARAM)
 *
 *  PURPOSE  :	Dialog proc for the frame and panes
 *
 *  ENTRY    :	HWND	hWndDlg;	// Dialog window handle
 *		UINT	msg;		// WM_xxx message
 *		WPARAM	wParam; 	// Message 16-bit parameter
 *		LPARAM	lParam; 	// Message 32-bit parameter
 *
 *  RETURNS  :	Non-zero - Message processed
 *		Zero	- DefDlgProc() must process the message
 *
 ****************************************************************************/

BOOL CALLBACK _export AdvancedMsgProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	//char	szBuf[128];
	//WINDOWPLACEMENT wndpl;

	switch (msg) {
//	case WM_ACTIVATEAPP:
//	    Globals.fCheckOnKillFocus = (BOOL) wParam;
//	    break;

//	case WM_ACTIVATE:
//	    Globals.fCheckOnKillFocus = (wParam != WA_INACTIVE);
//	    break;

	case WM_CLOSE:
	    {
//		if (IDCANCEL == CheckSave(hWnd)) break;

//		Globals.fCheckOnKillFocus = FALSE;	// Stop checking controls

//		if (Globals.nActiveDlg) {	// A dialog is still open
//		    SetFocus(hWnd);
//
//		    Globals.nActiveDlg = 0;
//		}

		EndDialog(hWnd, IDCANCEL);
//		Globals.nActiveDlg = 0;
	    }
	    break;

	//case WM_COMMAND:
	    //Pane_OnCommand(hWnd, (UINT) wParam, (HWND) LOWORD(lParam), (WORD) HIWORD(lParam));
	    //return (FALSE);

	#if 0
	case WM_CTLCOLOR:
	    {
		HDC	hDC = (HDC) wParam;
		HWND	hWndChild = (HWND) LOWORD(lParam);

		//if (GetDlgItem(hWnd, IDD_HELP) != hWndChild) return (NULL);

		switch ((UINT) HIWORD(lParam)) {
		    case CTLCOLOR_EDIT:
			SetBkColor(hDC, GetSysColor(COLOR_BTNFACE));
			return ((BOOL) (WORD) Globals.hbrHelp);

		    case CTLCOLOR_MSGBOX:
			return ((BOOL) (WORD) Globals.hbrHelp);
		}

		return (NULL);
	    }
#endif
	//case WM_QUERYENDSESSION:

		//memset(&wndpl, 0, sizeof(WINDOWPLACEMENT));
		//wndpl.length = sizeof(WINDOWPLACEMENT);
		//GetWindowPlacement(hWnd, &wndpl);

//		wsprintf( szBuf, "%d %d", wndpl.rcNormalPosition.left,
			//wndpl.rcNormalPosition.top );

		//WritePrivateProfileString( Globals.szAppName,	// Section name
//			"Position", (LPSTR) &szBuf, PROFILE );

		// this _should_ return 1, but Windows fails to close if it
		//   does!!
		//return 0;

	//case WM_DESTROY:
	    //{
//		char	szBuf[128];
		//WINDOWPLACEMENT wndpl;

		//if (Globals.hbrHelp) { DeleteBrush(Globals.hbrHelp);  Globals.hbrHelp = NULL; }

		//return (FALSE);
	    //}

	//case WM_SIZE:
	    //if (wParam == SIZE_MINIMIZED) {
			//SetWindowText(hWnd, APPNAME);
	    //} else if (wParam == SIZE_MAXIMIZED || wParam == SIZE_RESTORED) {
			//SetWindowText(hWnd, szWindowTitle);
	    //}

	    //return (FALSE);		// Requires default processing

	//case WM_ERASEBKGND:
	    //if (IsIconic(hWnd)) {
		//return (TRUE);		// Pretend we erased it
	    //} else {
		//return (FALSE); 	// We didn't erase anything
	    //}

	//case WM_INITDIALOG:
	    //return (Pane_OnInitDialog(hWnd, (HWND) wParam, lParam));

	//case WM_PAINT:
//		if (IsIconic(hWnd)) {
			//Pane_OnPaint_Iconic(hWnd);
		//} else {
		//	Pane_OnPaint(hWnd);
		//}
	    //return (FALSE);

	//case WM_SETFONT:
	    //Pane_OnSetFont(hWnd, (HFONT) wParam, (BOOL) LOWORD(lParam));
	    //return (FALSE);
	}

	return (FALSE);
}
