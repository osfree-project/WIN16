/*
 * 				Shell Library Functions
 *
 * Copyright 1993, 1994, 1995 Alexandre Julliard
 * Copyright 1997 Willows Software, Inc. 
 * Copyright 1998 Marcus Meissner
 * Copyright 2000 Juergen Schmied
 * Copyright 2002 Eric Pouech
 * Copyright 2023 Yuri Prokushev
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301, USA
 */

#include <string.h>
#include <stdio.h>

#include "windows.h"
#include "About.h"

#define TWIN_PLATFORM	"DOS"
#define GET_WM_COMMAND_ID(wp, lp)                   (wp)

typedef struct {
	LPSTR	lpszCaption;
	LPSTR	lpszText;
	HICON	hIcon;
} SHELLABOUTDATA, FAR * LPSHELLABOUTDATA;

static HINSTANCE hInst = 0;

BOOL CALLBACK AboutDlgProc(HWND, UINT, WPARAM, LPARAM);

/*************************************************************************
 * ShellAbout	[SHELL.22]
 *
 */
void WINAPI ShellAbout(HWND hWnd, LPCSTR lpszCaption, LPCSTR lpszAboutText, HICON hIcon)
{
	DLGPROC	lpProc;
	SHELLABOUTDATA sad;
	LPSHELLABOUTDATA lpsad;
	HINSTANCE  hIns;
	int     rc;
	
//	if(hWnd)
//		hIns = ((HMODULE)GetWindowWord(hWnd, GWW_HINSTANCE));
//	else    hIns = hInst;

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
	SHELLABOUTDATA FAR *sad;
	HWND	        hWnd;
	HDC	        hDC;
	char		abouttext[256];
	char		dirname[256];
	DWORD		version;
	int		bpp;

	switch(msg) {
	case WM_INITDIALOG:
		sad = (SHELLABOUTDATA FAR *) lParam;

		/************************************************/
		/*	Caption first				*/
		/************************************************/
		sprintf(abouttext,"About %s", sad->lpszCaption);
	    	SetWindowText(hDlg,abouttext);

		/************************************************/
		/*	Default first two lines			*/
		/************************************************/
		SendDlgItemMessage (hDlg, SAB_ABOUT, WM_SETTEXT, 0,
		                    ( LONG )sad->lpszCaption);
		SendDlgItemMessage (hDlg, SAB_TEXT, WM_SETTEXT, 0,
		                    ( LONG )sad->lpszText);

		/************************************************/
		/*	Twin specific 5 lines			*/
		/************************************************/
		GetModuleFileName(0,abouttext,256);


		/* add any shell about specific string */
		SendDlgItemMessage(hDlg, SAB_USER, WM_SETTEXT, 0,
			(LONG) abouttext);

		GetWindowsDirectory(dirname,256);
		sprintf(abouttext,"Windows: %s",dirname);
		SendDlgItemMessage(hDlg, SAB_WINDOW, WM_SETTEXT, 0,
			(LONG) abouttext);

		GetSystemDirectory(dirname,256);
		sprintf(abouttext,"System: %s",dirname);
		SendDlgItemMessage(hDlg, SAB_SYSTEM, WM_SETTEXT, 0,
			(LONG) abouttext);

		/************************************************/
		/*	Host specific 2 lines			*/
		/************************************************/
		/* host system information */
		sprintf ( abouttext, "Host: %s",TWIN_PLATFORM);
		SendDlgItemMessage(hDlg, SAB_HOST, WM_SETTEXT, 0,
			(LONG)abouttext);

		/* workstation information */
//		hDC = GetDC(hDlg);
//		bpp = GetDeviceCaps(hDC, BITSPIXEL);
//		ReleaseDC(hDlg, hDC);


		/* add the current mode... */
		version = GetVersion();
		sprintf(abouttext,"Version: Win16 %d.%d build %d) ",
			LOBYTE(LOWORD(version)), HIBYTE(LOWORD(version)),
			HIWORD(version)
			);

		SendDlgItemMessage(hDlg, SAB_VERSION, WM_SETTEXT, 0,
			(LONG)abouttext);

		/************************************************/
		/*	Icon passed in?				*/
		/************************************************/

		if (sad->hIcon) 
	    		SendDlgItemMessage(hDlg, SAB_ICON, STM_SETICON,
				sad->hIcon, 0L);
//		 else  {
//			hWnd = GetDlgItem(hDlg,SAB_ICON);
//			ShowWindow(hWnd,SW_HIDE);
//		}


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

extern  unsigned short          GetDS( void );
#pragma aux GetDS               = \
        "mov    ax,ds"          \
        value                   [ax];

#define GlobalPtrHandle(lp) \
  ((HGLOBAL)LOWORD(GlobalHandle(SELECTOROF(lp))))

#define     GlobalUnlockPtr(lp)      \
                GlobalUnlock(GlobalPtrHandle(lp))

#define GlobalFreePtr(lp) \
  (GlobalUnlockPtr(lp),(BOOL)GlobalFree(GlobalPtrHandle(lp)))

#define GlobalAllocPtr(flags, cb) \
  (GlobalLock(GlobalAlloc((flags), (cb))))

void lmemcpy(void far * s1, void far * s2, unsigned length);
int lstrnicmp(char far *s1, const char far *s2, int n);

/*************************************************************************
 * ExtractIcon	[SHELL.34]
 *
 */
HICON WINAPI
ExtractIcon(HINSTANCE hInst, LPCSTR lpszExeFileName, UINT nIconIndex)
{
//    APISTR((LF_APISTUB,"ExtractIcon(HINSTANCE=%x,LPCSTR=%s,UINT=%x0\n",
//	hInst,lpszExeFileName?lpszExeFileName:"NULL",nIconIndex));
	MessageBox(0, "ExtractIcon", "ExtractIcon", MB_OK);
    return (HICON)0;
}

/*************************************************************************
 *				ExtractAssociatedIcon	[SHELL.36]
 *
 * Return icon for given file (either from file itself or from associated
 * executable) and patch parameters if needed.
 */
HICON WINAPI ExtractAssociatedIcon(HINSTANCE hInst, LPSTR lpIconPath, LPWORD lpiIcon)
{
	MessageBox(0, "ExtractAssociatedIcon", "ExtractAssociatedIcon", MB_OK);
    return 0;//convert_icon_to_16( hInst, ExtractAssociatedIconA(NULL, lpIconPath, lpiIcon) );
}

/*************************************************************************
 * ShellExecute	[SHELL.20]
 *
 */
HINSTANCE WINAPI ShellExecute(HWND hWnd, LPCSTR lpOperation, LPCSTR lpFile,
		LPCSTR lpParameters, LPCSTR lpDirectory, int nShowCmd)
{
    HINSTANCE hInst;
    //APISTR((LF_APICALL,
      //"ShellExecute(HWND=%x,LPCSTR=%x,LPCSTR=%s,LPCSTR=%x,LPCSTR=%s,int=%d)\n",
	//hWnd,lpOperation,lpFile,lpParameters,lpDirectory,nShowCmd));
	MessageBox(0, "ShellExecute", "ShellExecute", MB_OK);

    hInst = WinExec(lpFile,nShowCmd);
    //APISTR((LF_APIRET,"ShellExecute: returns HINSTANCE %x\n",hInst));
    return hInst;
}

/*************************************************************************
 * FindExecutable	[SHELL.21]
 *
 */
HINSTANCE WINAPI FindExecutable(LPCSTR lpszFile, LPCSTR lpszDir, LPSTR lpResult)
{
    //APISTR((LF_APISTUB,"FindExecutable(LPCSTR=%s,LPCSTR=%s,LPSTR=%x)\n",
	//lpszFile,lpszDir,lpResult));
	MessageBox(0, "FindExecutable", "FindExecutable", MB_OK);
   /* later, add a search for the application to match file if not .exe */
    lstrcpy(lpResult,lpszFile);
    return (HINSTANCE) 33;
}


static const char lpstrMsgWndCreated[] = "OTHERWINDOWCREATED";
static const char lpstrMsgWndDestroyed[] = "OTHERWINDOWDESTROYED";
static const char lpstrMsgShellActivate[] = "ACTIVATESHELLWINDOW";

static HWND SHELL_hWnd=0;
static HHOOK SHELL_hHook=0;
static HOOKPROC SHELL_lpProc=NULL;

static UINT	uMsgWndCreated = 0;
static UINT	uMsgWndDestroyed = 0;
static UINT	uMsgShellActivate = 0;


/*************************************************************************
 * ShellHookProc		[SHELL.103]
 * System-wide WH_SHELL hook.
 */
LRESULT CALLBACK ShellHookProc(int code, WPARAM wParam, LPARAM lParam)
{
    if (SHELL_hWnd)
    {
        switch( code )
        {
        case HSHELL_WINDOWCREATED:
            PostMessage( SHELL_hWnd, uMsgWndCreated, wParam, 0 );
            break;
        case HSHELL_WINDOWDESTROYED:
            PostMessage( SHELL_hWnd, uMsgWndDestroyed, wParam, 0 );
            break;
        case HSHELL_ACTIVATESHELLWINDOW:
            PostMessage( SHELL_hWnd, uMsgShellActivate, wParam, 0 );
            break;
        }
    }
    return CallNextHookEx( SHELL_hHook, code, wParam, lParam );
}


/*************************************************************************
 * RegisterShellHook	[SHELL.102]
 *

From xoblite:
		RegisterShellHook(NULL, true);

                if(osInfo.dwPlatformId == VER_PLATFORM_WIN32_WINDOWS)
                        RegisterShellHook(hMainWnd, 1);
                else
                        RegisterShellHook(hMainWnd, 3);

-----------
        if (RegisterShellHook) RegisterShellHook(hMainWnd, 0);

From wine shell32.dll

// RegisterShellHook types
#define RSH_DEREGISTER          0
#define RSH_REGISTER            1
#define RSH_REGISTER_PROGMAN    2
#define RSH_REGISTER_TASKMAN    3

 */

#define RSH_DEREGISTER          0
#define RSH_REGISTER            1
#define RSH_REGISTER_PROGMAN    2
#define RSH_REGISTER_TASKMAN    3

BOOL WINAPI RegisterShellHook(HWND hWnd, UINT uAction)
{
	MessageBox(0, "RegisterShellHook", "RegisterShellHook", MB_OK);
    //APISTR((LF_APISTUB,"RegisterShellHook(HWND=%x,int=%d)\n",hWnd,foo));

    switch( uAction )
    {
    case RSH_DEREGISTER:
        if (SHELL_hHook)
        {
		UnhookWindowsHook(WH_SHELL, (HOOKPROC)SHELL_lpProc);
		FreeProcInstance(SHELL_lpProc);
		SHELL_hHook = 0;
		return TRUE;
        }
	break;
    case RSH_REGISTER:
        break;
    case RSH_REGISTER_PROGMAN:  /* register hWnd as a shell window */
        if( !SHELL_hHook )
        {
		SHELL_lpProc= MakeProcInstance((FARPROC)ShellHookProc, hInst);
            SHELL_hHook = SetWindowsHookEx( WH_SHELL, SHELL_lpProc, hInst, 0 );
            if ( SHELL_hHook )
            {
                uMsgWndCreated = RegisterWindowMessage( lpstrMsgWndCreated );
                uMsgWndDestroyed = RegisterWindowMessage( lpstrMsgWndDestroyed );
                uMsgShellActivate = RegisterWindowMessage( lpstrMsgShellActivate );
            }
            else  
                MessageBox(0, "-- unable to install ShellHookProc()!", "", MB_OK);
        }

        if ( SHELL_hHook )
            return ((SHELL_hWnd = hWnd) != 0);
        break;

    case RSH_REGISTER_TASKMAN:
        break;

    default:
//        WARN("-- unknown code %i\n", uAction );
        SHELL_hWnd = 0; /* just in case */
    }
    return FALSE;
}


/* (WIN32) File Security *************************************************** */
#if 0   
BOOL    WINAPI
GetFileSecurity(LPCSTR lpszFile,
        SECURITY_INFORMATION si, PSECURITY_DESCRIPTOR psd,
        DWORD dw, LPDWORD lp)
{
        //APISTR((LF_APISTUB, "GetFileSecurity(LPCSTR=%s,...)\n", lpszFile));
        return (TRUE);
}       
   
BOOL    WINAPI
SetFileSecurity(LPCSTR lpszFile,
        SECURITY_INFORMATION si, PSECURITY_DESCRIPTOR psd)
{   
        //APISTR((LF_APISTUB, "SetFileSecurity(LPCSTR=%s,...)\n", lpszFile));
        return (TRUE);
}
#endif

/***********************************************************************
 * DllEntryPoint [SHELL.101]
 *
 * Initialization code for shell.dll. Automatically loads the
 * 32-bit shell32.dll to allow thunking up to 32-bit code.
 *
 * RETURNS
 *  Success: TRUE. Initialization completed successfully.
 *  Failure: FALSE.
 */
BOOL WINAPI DllEntryPoint(DWORD Reason, HINSTANCE hInst,
				WORD ds, WORD HeapSize, DWORD res1, WORD res2)
{
    return TRUE;
}

/*************************************************************************
 *				FindEnvironmentString	[SHELL.38]
 *
 * Returns a pointer into the DOS environment... Ugh.
 */
static LPSTR SHELL_FindString(LPSTR lpEnv, LPCSTR entry)
{ UINT l;

//  TRACE("\n");

  l = lstrlen(entry);
  for( ; *lpEnv ; lpEnv+=lstrlen(lpEnv)+1 )
  { if( lstrnicmp(lpEnv, entry, l) )
      continue;
	if( !*(lpEnv+l) )
	    return (lpEnv + l); 		/* empty entry */
	else if ( *(lpEnv+l)== '=' )
	    return (lpEnv + l + 1);
    }
    return NULL;
}

/**********************************************************************/

LPSTR WINAPI FindEnvironmentString(LPCSTR str)
{ LPSTR  spEnv;
  LPSTR lpEnv,lpString;
  //TRACE("\n");

  lpEnv = GetDOSEnvironment();

  lpString = (lpEnv)?SHELL_FindString(lpEnv, str):NULL;

    if( lpString )
	return lpString;

    return 0;
}

/*************************************************************************
 * DoEnvironmentSubst      [SHELL.37]
 *
 * Replace %KEYWORD% in the str with the value of variable KEYWORD
 * from "DOS" environment. If it is not found the %KEYWORD% is left
 * intact. If the buffer is too small, str is not modified.
 *
 * PARAMS
 *  str        [I] '\0' terminated string with %keyword%.
 *             [O] '\0' terminated string with %keyword% substituted.
 *  length     [I] size of str.
 *
 * RETURNS
 *  str length in the LOWORD and 1 in HIWORD if subst was successful.
 */
DWORD WINAPI DoEnvironmentSubst(LPSTR str,WORD length)
{
  LPSTR   lpEnv = GetDOSEnvironment();
  LPSTR   lpstr = str;
  LPSTR   lpend;
  LPSTR   lpBuffer = GlobalAllocPtr( GPTR, length);
  WORD    bufCnt = 0;
  WORD    envKeyLen;
  LPSTR   lpKey;
  WORD    retStatus = 0;
  WORD    retLength = length;

	MessageBox(0, "DoEnvironmentSubst", "DoEnvironmentSubst", MB_OK);
//  CharToOemA(str,str);

//  TRACE("accept %s\n", str);

  while( *lpstr && bufCnt <= length - 1 ) {
     if ( *lpstr != '%' ) {
        lpBuffer[bufCnt++] = *lpstr++;
        continue;
     }

     for( lpend = lpstr + 1; *lpend && *lpend != '%'; lpend++) /**/;

     envKeyLen = lpend - lpstr - 1;
     if( *lpend != '%' || envKeyLen == 0)
        goto err; /* "%\0" or "%%" found; back off and whine */

     *lpend = '\0';
     lpKey = SHELL_FindString(lpEnv, lpstr+1);
     *lpend = '%';
     if( lpKey ) {
         int l = lstrlen(lpKey);

         if( bufCnt + l > length - 1 )
                goto err;

        lmemcpy(lpBuffer + bufCnt, lpKey, l);
        bufCnt += l;
     } else { /* Keyword not found; Leave the %KEYWORD% intact */
        if( bufCnt + envKeyLen + 2 > length - 1 )
            goto err;

         lmemcpy(lpBuffer + bufCnt, lpstr, envKeyLen + 2);
        bufCnt += envKeyLen + 2;
     }

     lpstr = lpend + 1;
  }

  if (!*lpstr && bufCnt <= length - 1) {
      lmemcpy(str,lpBuffer, bufCnt);
      str[bufCnt] = '\0';
      retLength = bufCnt + 1;
      retStatus = 1;
  }

  err:
//  if (!retStatus)
//      WARN("-- Env subst aborted - string too short or invalid input\n");
//  TRACE("-- return %s\n", str);

//  OemToCharA(str,str);
  GlobalFreePtr(lpBuffer);

  return (DWORD)MAKELONG(retLength, retStatus);
}

#pragma off (unreferenced);
BOOL FAR PASCAL LibMain( HINSTANCE hInstance, WORD wDataSegment, WORD wHeapSize, LPSTR lpszCmdLine )
#pragma on (unreferenced);
{
	if (!hInst)
		hInst=hInstance;

	return(1);
}

#pragma off (unreferenced);
int FAR PASCAL WEP( int nParameter )
#pragma on (unreferenced);
{
  return( 1 );
}


/*************************************************************************
 *			InternalExtractIcon		[SHELL.39]
 *
 * This abortion is called directly by Progman
 */
HGLOBAL WINAPI InternalExtractIcon(HINSTANCE hInstance, LPCSTR lpszExeFileName, UINT nIconIndex, WORD n )
{
    HGLOBAL hRet = 0;
    HICON FAR * RetPtr = NULL;

//	TRACE("(%04x,file %s,start %d,extract %d\n",
//		       hInstance, lpszExeFileName, nIconIndex, n);

	MessageBox(0, "InternalExtractIcon", "InternalExtractIcon", MB_OK);	
//	if (!n)
	  return 0;

	hRet = GlobalAlloc(GMEM_FIXED | GMEM_ZEROINIT, sizeof(*RetPtr) * n);
        RetPtr = (HICON FAR *)GlobalLock(hRet);

	if (nIconIndex == (UINT)-1)  /* get number of icons */
	{
//	  RetPtr[0] = PrivateExtractIconsA(lpszExeFileName, 0, 0, 0, NULL, NULL, 0, LR_DEFAULTCOLOR);
	}
	else
	{
	  UINT ret;
	  HICON FAR *icons;

	  icons = (HICON FAR *)GlobalAllocPtr(GPTR, n * sizeof(*icons));
//	  ret = PrivateExtractIconsA(lpszExeFileName, nIconIndex,
//	                             GetSystemMetrics(SM_CXICON),
//	                             GetSystemMetrics(SM_CYICON),
//	                             icons, NULL, n, LR_DEFAULTCOLOR);
	  if ((ret != 0xffffffff) && ret)
	  {
	    int i;
//	    for (i = 0; i < n; i++) RetPtr[i] = convert_icon_to_16(hInstance, icons[i]);
	  }
	  else
	  {
	    GlobalFree(hRet);
	    hRet = 0;
	  }
	  GlobalFreePtr(icons);
	}
	return hRet;
}
