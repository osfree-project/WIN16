#include "Shell.h"

/*************************************************************************
 * ShellAbout	[SHELL.22]
 *
 */
void WINAPI ShellAbout(HWND hWnd, LPCSTR lpszCaption, LPCSTR lpszAboutText, HICON hIcon)
{
	DLGPROC	lpProc;
	SHELLABOUTDATA sad;
	LPSHELLABOUTDATA lpsad;
	int     rc;
	
	lpProc= MakeProcInstance((FARPROC)AboutDlgProc, hInst);

	sad.lpszText    = (LPSTR)lpszAboutText;
	sad.lpszCaption = (LPSTR)lpszCaption;
	sad.hIcon = hIcon;

	lpsad=&sad;

	rc = DialogBoxParam(
		hInst,
		MAKEINTRESOURCE(IDD_SHELLABOUT),
		hWnd,
		lpProc,
		(LPARAM)lpsad
		);

	FreeProcInstance(lpProc); 
}

BOOL CALLBACK AboutDlgProc(HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	LPSHELLABOUTDATA sad;
	HWND		hWnd;
	HDC			hDC;
	char		abouttext[256];
	char		buf[256];
	char		dirname[256];
	LPSTR		lpszTmp = NULL;
	DWORD		version;
	int			bpp;

	switch(msg) {
	case WM_INITDIALOG:
		sad = (LPSHELLABOUTDATA) lParam;

		/************************************************/
		/*	Caption first				*/
		/************************************************/
		if (lpszTmp=lstrchr(sad->lpszCaption, '#'))
		{
			*lpszTmp='\0';
		    	SetWindowText(hDlg,sad->lpszCaption);
			lpszTmp++;
			sad->lpszCaption=lpszTmp;
		} else {
			GetWindowText(hDlg, buf, sizeof(buf));
			sprintf(abouttext, buf, sad->lpszCaption);
		    	SetWindowText(hDlg, abouttext);
		}

		/************************************************/
		/*	Default first two lines			*/
		/************************************************/
		GetDlgItemText(hDlg, SAB_ABOUT, buf, sizeof(buf));
		sprintf(abouttext, buf, sad->lpszCaption);
		SetDlgItemText(hDlg, SAB_ABOUT, abouttext);

		SetDlgItemText(hDlg, SAB_TEXT, sad->lpszText);

		/************************************************/
		/*	Twin specific 5 lines			*/
		/************************************************/
		GetModuleFileName(0,abouttext,256);

		/* add any shell about specific string */
		SetDlgItemText(hDlg, SAB_USER, abouttext);

		GetWindowsDirectory(dirname,256);
		sprintf(abouttext,"Windows: %s",dirname);
		SetDlgItemText(hDlg, SAB_WINDOW, abouttext);

		GetSystemDirectory(dirname,256);
		sprintf(abouttext,"System: %s",dirname);
		SetDlgItemText(hDlg, SAB_SYSTEM, abouttext);

		/************************************************/
		/*	Host specific 2 lines			*/
		/************************************************/
		/* host system information */
		sprintf ( abouttext, "Host: DOS");
		SetDlgItemText(hDlg, SAB_HOST, abouttext);

		/* add the current mode... */
		version = GetVersion();
		sprintf(abouttext,"Version: Win16 %d.%d build %d) ",
			LOBYTE(LOWORD(version)), HIBYTE(LOWORD(version)),
			HIWORD(version)
			);

		SetDlgItemText(hDlg, SAB_VERSION, abouttext);

		/************************************************/
		/*	Icon passed in?				*/
		/************************************************/

		if (sad->hIcon) 
	    		SendDlgItemMessage(hDlg, SAB_ICON, STM_SETICON,
				sad->hIcon, 0L);

		return TRUE;

	case WM_COMMAND:
		switch ( GET_WM_COMMAND_ID(wParam, lParam) )
		{
			case IDOK:
				EndDialog(hDlg, IDOK);
				break;
			default:
				return ( FALSE );
		}
		return ( TRUE );
	}
	return FALSE;
}
