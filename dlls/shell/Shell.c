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
#include "Shell.h"

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
//	HINSTANCE  hIns;
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

#pragma pack(push, 1)

#define IMAGE_DOS_SIGNATURE     0x5A4D
#define IMAGE_OS2_SIGNATURE     0x454E

/* Header shared by DOS, Win16, Win32, and Win64 executables */
typedef struct _IMAGE_DOS_HEADER {
    WORD    e_magic;
    WORD    e_cblp;
    WORD    e_cp;
    WORD    e_crlc;
    WORD    e_cparhdr;
    WORD    e_minalloc;
    WORD    e_maxalloc;
    WORD    e_ss;
    WORD    e_sp;
    WORD    e_csum;
    WORD    e_ip;
    WORD    e_cs;
    WORD    e_lfarlc;
    WORD    e_ovno;
    WORD    e_res[4];
    WORD    e_oemid;
    WORD    e_oeminfo;
    WORD    e_res2[10];
    LONG    e_lfanew;
} IMAGE_DOS_HEADER;
typedef IMAGE_DOS_HEADER FAR * LPIMAGE_DOS_HEADER;

/* Header for OS/2 executables */
typedef struct _IMAGE_OS2_HEADER {
    WORD    ne_magic;
    char    ne_ver;
    char    ne_rev;
    WORD    ne_enttab;
    WORD    ne_cbenttab;
    LONG    ne_crc;
    WORD    ne_flags;
    WORD    ne_autodata;
    WORD    ne_heap;
    WORD    ne_stack;
    LONG    ne_csip;
    LONG    ne_sssp;
    WORD    ne_cseg;
    WORD    ne_cmod;
    WORD    ne_cbnrestab;
    WORD    ne_segtab;
    WORD    ne_rsrctab;
    WORD    ne_restab;
    WORD    ne_modtab;
    WORD    ne_imptab;
    LONG    ne_nrestab;
    WORD    ne_cmovent;
    WORD    ne_align;
    WORD    ne_cres;
    BYTE    ne_exetyp;
    BYTE    ne_flagsothers;
    WORD    ne_pretthunks;
    WORD    ne_psegrefbytes;
    WORD    ne_swaparea;
    WORD    ne_expver;
} IMAGE_OS2_HEADER;
typedef IMAGE_OS2_HEADER FAR * LPIMAGE_OS2_HEADER;

typedef struct
{
    BYTE        bWidth;          /* Width, in pixels, of the image	*/
    BYTE        bHeight;         /* Height, in pixels, of the image	*/
    BYTE        bColorCount;     /* Number of colors in image (0 if >=8bpp) */
    BYTE        bReserved;       /* Reserved ( must be 0)		*/
    WORD        wPlanes;         /* Color Planes			*/
    WORD        wBitCount;       /* Bits per pixel			*/
    DWORD       dwBytesInRes;    /* How many bytes in this resource?	*/
    DWORD       dwImageOffset;   /* Where in the file is this image?	*/
} icoICONDIRENTRY, FAR * LPicoICONDIRENTRY;

typedef struct
{
    WORD            idReserved;   /* Reserved (must be 0) */
    WORD            idType;       /* Resource Type (RES_ICON or RES_CURSOR) */
    WORD            idCount;      /* How many images */
    icoICONDIRENTRY idEntries[1]; /* An entry for each image (idCount of 'em) */
} icoICONDIR, FAR * LPicoICONDIR;

typedef struct
{
    WORD offset;
    WORD length;
    WORD flags;
    WORD id;
    WORD handle;
    WORD usage;
} NE_NAMEINFO;

typedef struct
{
    WORD  type_id;
    WORD  count;
    DWORD resloader;
} NE_TYPEINFO;

#define NE_RSCTYPE_ICON        0x8003
#define NE_RSCTYPE_GROUP_ICON  0x800e

typedef struct
{
    BYTE   bWidth;
    BYTE   bHeight;
    BYTE   bColorCount;
    BYTE   bReserved;
} ICONRESDIR;

typedef struct
{
    WORD   wWidth;
    WORD   wHeight;
} CURSORDIR;

typedef struct
{   union
    { ICONRESDIR icon;
      CURSORDIR  cursor;
    } ResInfo;
    WORD   wPlanes;
    WORD   wBitCount;
    DWORD  dwBytesInRes;
    WORD   wResId;
} CURSORICONDIRENTRY;

typedef struct
{
    WORD                idReserved;
    WORD                idType;
    WORD                idCount;
    CURSORICONDIRENTRY  idEntries[1];
} CURSORICONDIR;

typedef struct {
    BYTE bWidth;
    BYTE bHeight;
    BYTE bColorCount;
    BYTE bReserved;
    WORD xHotspot;
    WORD yHotspot;
    DWORD dwDIBSize;
    DWORD dwDIBOffset;
} CURSORICONFILEDIRENTRY;

typedef struct
{
    WORD                idReserved;
    WORD                idType;
    WORD                idCount;
    CURSORICONFILEDIRENTRY  idEntries[1];
} CURSORICONFILEDIR;

#pragma pack(pop)

/*************************************************************************
 *                      ICO_GetIconDirectory
 *
 * Reads .ico file and build phony ICONDIR struct
 */
static BYTE FAR * ICO_GetIconDirectory( LPBYTE peimage, LPicoICONDIR FAR * lplpiID, ULONG *uSize )
{
	CURSORICONFILEDIR FAR *lpcid;	/* icon resource in resource-dir format */
	CURSORICONDIR FAR * lpID;		/* icon resource in resource format */
	int		i;

	//TRACE("%p %p\n", peimage, lplpiID);

	lpcid = (CURSORICONFILEDIR FAR *)peimage;

	if( lpcid->idReserved || (lpcid->idType != 1) || (!lpcid->idCount) )
	  return 0;

	/* allocate the phony ICONDIR structure */
        *uSize = FIELD_OFFSET(CURSORICONDIR, idEntries[lpcid->idCount]);
	if( (lpID = GlobalAllocPtr(GPTR, *uSize) ))
	{
	  /* copy the header */
	  lpID->idReserved = lpcid->idReserved;
	  lpID->idType = lpcid->idType;
	  lpID->idCount = lpcid->idCount;

	  /* copy the entries */
	  for( i=0; i < lpcid->idCount; i++ )
	  {
            lmemcpy(&lpID->idEntries[i], &lpcid->idEntries[i], sizeof(CURSORICONDIRENTRY) - 2);
	    lpID->idEntries[i].wResId = i;
	  }

	  *lplpiID = (LPicoICONDIR)peimage;
	  return (BYTE *)lpID;
	}
	return 0;
}

/*************************************************************************
 *                      ICO_LoadIcon
 */
static BYTE FAR * ICO_LoadIcon( LPBYTE peimage, LPicoICONDIRENTRY lpiIDE, ULONG FAR * uSize)
{
//	TRACE("%p %p\n", peimage, lpiIDE);

	*uSize = lpiIDE->dwBytesInRes;
	return peimage + lpiIDE->dwImageOffset;
}

/*************************************************************************
 *				USER32_GetResourceTable
 */
static DWORD USER32_GetResourceTable(LPBYTE peimage,DWORD pesize,LPBYTE FAR *retptr)
{
	LPIMAGE_DOS_HEADER	mz_header;

//	TRACE("%p %p\n", peimage, retptr);

	*retptr = NULL;

	mz_header = (LPIMAGE_DOS_HEADER) peimage;

	if (mz_header->e_magic != IMAGE_DOS_SIGNATURE)
	{
	  if (mz_header->e_cblp == 1)	/* .ICO file ? */
	  {
	    *retptr = (LPBYTE)-1;	/* ICONHEADER.idType, must be 1 */
	    return 1;
	  }
	  else
	    return 0; /* failed */
	}
	if (mz_header->e_lfanew >= pesize) {
	    return 0; /* failed, happens with PKZIP DOS Exes for instance. */
	}
	//if (*((DWORD*)(peimage + mz_header->e_lfanew)) == IMAGE_NT_SIGNATURE )
//	  return IMAGE_NT_SIGNATURE;

	if (*((WORD FAR *)(peimage + mz_header->e_lfanew)) == IMAGE_OS2_SIGNATURE )
	{
	  LPIMAGE_OS2_HEADER	ne_header;

	  ne_header = (LPIMAGE_OS2_HEADER)(peimage + mz_header->e_lfanew);

	  if (ne_header->ne_magic != IMAGE_OS2_SIGNATURE)
	    return 0;

	  if( (ne_header->ne_restab - ne_header->ne_rsrctab) <= sizeof(NE_TYPEINFO) )
	    *retptr = (LPBYTE)-1;
	  else
	    *retptr = peimage + mz_header->e_lfanew + ne_header->ne_rsrctab;

	  return IMAGE_OS2_SIGNATURE;
	}
	return 0; /* failed */
}

/***********************************************************************
 *           PrivateExtractIconsA			[USER32.@]
 */

UINT WINAPI PrivateExtractIconsA (
	LPCSTR lpszExeFileName,
	int nIconIndex,
	UINT cxDesired,
	UINT cyDesired,
	HICON * RetPtr, /* [out] pointer to array of nIcons HICON handles */
	UINT * pIconId,  /* [out] pointer to array of nIcons icon identifiers or NULL */
	UINT nIcons,    /* [in] number of icons to retrieve */
	UINT flags )    /* [in] LR_* flags used by LoadImage */
{
//	TRACE("%s %d %dx%d %p %p %d 0x%08x\n",
//	      debugstr_w(lpwstrFile), nIndex, sizeX, sizeY, phicon, pIconId, nIcons, flags);

//	if ((nIcons & 1) && HIWORD(sizeX) && HIWORD(sizeY))
//	{
//	  WARN("Uneven number %d of icons requested for small and large icons!\n", nIcons);
	//}
	//return ICO_ExtractIconExW(lpwstrFile, phicon, nIndex, nIcons, sizeX, sizeY, pIconId, flags);
/*************************************************************************
 *	ICO_ExtractIconExW		[internal]
 *
 * NOTES
 *  nIcons = 0: returns number of Icons in file
 *
 * returns
 *  invalid file: -1
 *  failure:0;
 *  success: number of icons in file (nIcons = 0) or nr of icons retrieved
 */
//static UINT ICO_ExtractIconExW(
//	LPCSTR lpszExeFileName,
//	HICON * RetPtr,
//	INT nIconIndex,
//	UINT nIcons,
//	UINT cxDesired,
//	UINT cyDesired,
//	UINT *pIconId,
//	UINT flags)
//{
	UINT		ret = 0;
	UINT		cx1, cx2, cy1, cy2;
	LPBYTE		pData;
	DWORD		sig;
	HANDLE		hFile;
	UINT		iconDirCount = 0,iconCount = 0;
	LPBYTE		peimage;
	HANDLE		fmapping;
	DWORD		fsizeh,fsizel;
    char		szExePath[MAX_PATH];
    DWORD		dwSearchReturn;

//	TRACE("%s, %d, %d %p 0x%08x\n", debugstr_w(lpszExeFileName), nIconIndex, nIcons, pIconId, flags);

        dwSearchReturn = SearchPath(NULL, lpszExeFileName, NULL, ARRAY_SIZE(szExePath), szExePath, NULL);
        if ((dwSearchReturn == 0) || (dwSearchReturn > ARRAY_SIZE(szExePath)))
        {
            //WARN("File %s not found or path too long\n", debugstr_w(lpszExeFileName));
            return -1;
        }

	hFile = CreateFile(szExePath, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, 0);
	if (hFile == INVALID_HANDLE_VALUE) return ret;
	fsizel = GetFileSize(hFile,&fsizeh);

	/* Map the file */
	fmapping = CreateFileMapping(hFile, NULL, PAGE_READONLY | SEC_COMMIT, 0, 0, NULL);
	CloseHandle(hFile);
	if (!fmapping)
	{
          //WARN("CreateFileMapping error %ld\n", GetLastError() );
	  return 0xFFFF;
	}

	if (!(peimage = MapViewOfFile(fmapping, FILE_MAP_READ, 0, 0, 0)))
	{
          //WARN("MapViewOfFile error %ld\n", GetLastError() );
	  CloseHandle(fmapping);
	  return 0xFFFF;
	}
	CloseHandle(fmapping);

	cx1 = LOWORD(cxDesired);
	cx2 = HIWORD(cxDesired);
	cy1 = LOWORD(cyDesired);
	cy2 = HIWORD(cyDesired);

	if (pIconId) /* Invalidate first icon identifier */
		*pIconId = 0xFFFF;

	if (!pIconId) /* if no icon identifier array present use the icon handle array as intermediate storage */
	  pIconId = (UINT*)RetPtr;

	sig = USER32_GetResourceTable(peimage, fsizel, &pData);

/* ico file or NE exe/dll*/
	if (sig==IMAGE_OS2_SIGNATURE || sig==1) /* .ICO file */
	{
	  BYTE		*pCIDir = 0;
	  NE_TYPEINFO	FAR * pTInfo = (NE_TYPEINFO FAR *)(pData + 2);
	  NE_NAMEINFO	*pIconStorage = NULL;
	  NE_NAMEINFO	*pIconDir = NULL;
	  LPicoICONDIR	lpiID = NULL;
	  ULONG		uSize = 0;

          //TRACE("-- OS2/icon Signature (0x%08lx)\n", sig);

	  if (pData == (BYTE FAR *)-1)
	  {
	    pCIDir = ICO_GetIconDirectory(peimage, &lpiID, &uSize);	/* check for .ICO file */
	    if (pCIDir)
	    {
	      iconDirCount = 1; iconCount = lpiID->idCount;
              //TRACE("-- icon found %p 0x%08lx 0x%08x 0x%08x\n", pCIDir, uSize, iconDirCount, iconCount);
	    }
	  }
	  else while (pTInfo->type_id && !(pIconStorage && pIconDir))
	  {
	    if (pTInfo->type_id == NE_RSCTYPE_GROUP_ICON)	/* find icon directory and icon repository */
	    {
	      iconDirCount = pTInfo->count;
	      pIconDir = ((NE_NAMEINFO*)(pTInfo + 1));
	      //TRACE("\tfound directory - %i icon families\n", iconDirCount);
	    }
	    if (pTInfo->type_id == NE_RSCTYPE_ICON)
	    {
	      iconCount = pTInfo->count;
	      pIconStorage = ((NE_NAMEINFO*)(pTInfo + 1));
	      //TRACE("\ttotal icons - %i\n", iconCount);
	    }
	    pTInfo = (NE_TYPEINFO *)((char*)(pTInfo+1)+pTInfo->count*sizeof(NE_NAMEINFO));
	  }

	  if ((pIconStorage && pIconDir) || lpiID)	  /* load resources and create icons */
	  {
	    if (nIcons == 0)
	    {
	      ret = iconDirCount;
              if (lpiID)	/* *.ico file, deallocate heap pointer*/
	        HeapFree(GetProcessHeap(), 0, pCIDir);
	    }
	    else if (nIconIndex < iconDirCount)
	    {
	      UINT   i, icon;
	      if (nIcons > iconDirCount - nIconIndex)
	        nIcons = iconDirCount - nIconIndex;

	      for (i = 0; i < nIcons; i++)
	      {
	        /* .ICO files have only one icon directory */
	        if (lpiID == NULL)	/* not *.ico */
	          pCIDir = USER32_LoadResource(peimage, pIconDir + i + nIconIndex, *(WORD*)pData, &uSize);
	        pIconId[i] = LookupIconIdFromDirectoryEx(pCIDir, TRUE, cx1, cy1, flags);
                if (cx2 && cy2) pIconId[++i] = LookupIconIdFromDirectoryEx(pCIDir, TRUE,  cx2, cy2, flags);
	      }
              if (lpiID)	/* *.ico file, deallocate heap pointer*/
	        HeapFree(GetProcessHeap(), 0, pCIDir);

	      for (icon = 0; icon < nIcons; icon++)
	      {
	        pCIDir = NULL;
	        if (lpiID)
	          pCIDir = ICO_LoadIcon(peimage, lpiID->idEntries + (int)pIconId[icon], &uSize);
	        else
	          for (i = 0; i < iconCount; i++)
	            if (pIconStorage[i].id == ((int)pIconId[icon] | 0x8000) )
	              pCIDir = USER32_LoadResource(peimage, pIconStorage + i, *(WORD*)pData, &uSize);

	        if (pCIDir)
                {
	          RetPtr[icon] = CreateIconFromResourceEx(pCIDir, uSize, TRUE, 0x00030000,
                                                                 cx1, cy1, flags);
                  if (cx2 && cy2)
                      RetPtr[++icon] = CreateIconFromResourceEx(pCIDir, uSize, TRUE, 0x00030000,
                                                                       cx2, cy2, flags);
                }
	        else
	          RetPtr[icon] = 0;
	      }
	      ret = icon;	/* return number of retrieved icons */
	    }
	  }
	}
/* end ico file */

end:
	UnmapViewOfFile(peimage);	/* success */
	return ret;

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
	  RetPtr[0] = PrivateExtractIconsA(lpszExeFileName, 0, 0, 0, NULL, NULL, 0, LR_DEFAULTCOLOR);
	}
	else
	{
	  UINT ret;
	  HICON FAR *icons;

	  icons = (HICON FAR *)GlobalAllocPtr(GPTR, n * sizeof(*icons));
	  ret = PrivateExtractIconsA(lpszExeFileName, nIconIndex,
	                             GetSystemMetrics(SM_CXICON),
	                             GetSystemMetrics(SM_CYICON),
	                             icons, NULL, n, LR_DEFAULTCOLOR);
	  if ((ret != 0xffff) && ret)
	  {
	    int i;
	    for (i = 0; i < n; i++) RetPtr[i] = icons[i];
	  } else
	  {
	    GlobalFree(hRet);
	    hRet = 0;
	  }
	  GlobalFreePtr(icons);
	}
	return hRet;
}
