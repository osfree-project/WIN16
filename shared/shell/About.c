/*
 * 				Shell Library Functions
 *
 * Copyright 1997 Willows Software, Inc. 
 * Copyright 2023 Yuri Prokushev
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
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

/*! @file About.c
    @brief ShellAbout dialog implementation.

    Two build targets:

    WINVER >= 0x030A:
        SHELL.DLL carries the dialog resource IDD_SHELLABOUT (see the
        .rc file), the string IDS_VERSION, and the icon SAB_ICON.
        ShellAbout() uses DialogBoxParam(MAKEINTRESOURCE(...)).

    WINVER < 0x030A:
        There is no SHELL.DLL on Windows 3.0 and no resource is
        carried by shell.lib, so the same layout is built
        programmatically as a DLGTEMPLATE and passed to
        DialogBoxIndirectParam. The template is allocated with
        GlobalAlloc and freed after the dialog closes (Win16
        DialogBoxIndirectParam expects an HGLOBAL, not a pointer).
        Strings are compiled in. The SAB_ICON control starts empty
        and is filled with the system default application icon
        (IDI_APPLICATION).

    The '#' separator handling and the title template are the same in
    both builds: the title is read from the dialog with GetWindowText
    and the caption is substituted into it, exactly as the original
    Win16 SHELL.DLL ordinal 022 did.
*/

#include "Shell.h"

/* ------------------------------------------------------------------ */
/* Windows 3.0: programmatic dialog template                          */
/* ------------------------------------------------------------------ */

#if WINVER < 0x030A

/*! @brief Append a WORD to a DLGTEMPLATE buffer, advance pointer. */
static LPSTR DlgTplPutWord(LPSTR p, WORD w)
{
	*(WORD FAR *)p = w;
	return p + sizeof(WORD);
}

/*! @brief Append a SHORT to a DLGTEMPLATE buffer, advance pointer. */
static LPSTR DlgTplPutShort(LPSTR p, short s)
{
	*(short FAR *)p = s;
	return p + sizeof(short);
}

/*! @brief Append a DWORD to a DLGTEMPLATE buffer, advance pointer. */
static LPSTR DlgTplPutDword(LPSTR p, DWORD d)
{
	*(DWORD FAR *)p = d;
	return p + sizeof(DWORD);
}

/*! @brief Append a NUL-terminated string to a DLGTEMPLATE buffer. */
static LPSTR DlgTplPutStr(LPSTR p, LPCSTR s)
{
	while (*s) *p++ = *s++;
	*p++ = '\0';
	return p;
}

/*! @brief Append one DLGITEMTEMPLATE and its class/text to a buffer. */
static LPSTR DlgTplAddItem(LPSTR p, short x, short y, short cx, short cy,
                            WORD id, DWORD style, WORD classAtom, LPCSTR text)
{
	p = DlgTplPutShort(p, x);
	p = DlgTplPutShort(p, y);
	p = DlgTplPutShort(p, cx);
	p = DlgTplPutShort(p, cy);
	p = DlgTplPutWord(p, id);
	p = DlgTplPutDword(p, style);
	p = DlgTplPutWord(p, 0xFFFF);	/* class field: use atom */
	p = DlgTplPutWord(p, classAtom);
	p = DlgTplPutStr(p, text);
	return p;
}

/*! @brief Build the About dialog template in a global block.

    Reproduces the layout of the Win 3.1 .rc:

        DIALOG 20,20,220,158
        CAPTION "About %s"
        STYLE DS_MODALFRAME | WS_POPUPWINDOW | WS_DLGFRAME
        BEGIN
            DEFPUSHBUTTON "OK", SAB_OKAY, 184, 5, 40, 14
            ICON SAB_ICON, SAB_ICON, 10, 3, 18, 16
            LTEXT "osFree Janus %s", SAB_ABOUT, 45, 5, 127, 10
            LTEXT "", SAB_VERSION, 45, 15, 100, 10
            LTEXT "Copyright (c) 2023 osFree", -1, 45, 25, 145, 10
            LTEXT "", SAB_TEXT, 45, 35, 145, 20
            CONTROL "", -1, "Static", SS_BLACKRECT, 45, 85, 172, 1
            LTEXT "Static", SAB_USER, 46, 55, 180, 20
            LTEXT "Static", SAB_WINDOW, 46, 65, 150, 10
            LTEXT "Host:", SAB_HOST, 46, 75, 150, 10
            LTEXT "Display:", SAB_TERM, 46, 85, 150, 10
        END

    The dialog title is "About %s", so the same GetWindowText +
    wsprintf path as on Win 3.1 applies in WM_INITDIALOG.

    Standard class atoms in Win16 DLGTEMPLATE:
        0x0080 - BUTTON
        0x0082 - STATIC

    Source for the HGLOBAL requirement: Windows 3.0 SDK, 
    DialogBoxIndirectParam(): "hDialogTemplate: Identifies a global
    memory block containing a DLGTEMPLATE structure".

    @return Handle to a movable global block holding the template,
            or NULL on failure. The caller must GlobalFree() the
            handle after DialogBoxIndirectParam() returns.
*/
static HGLOBAL BuildAboutTemplate(void)
{
	HGLOBAL hTpl;
	LPSTR p;
	LPSTR base;
	DLGTEMPLATE FAR *tpl;

	hTpl = GlobalAlloc(GMEM_MOVEABLE | GMEM_ZEROINIT, 1024);
	if (!hTpl)
		return NULL;

	base = (LPSTR)GlobalLock(hTpl);
	if (!base) {
		GlobalFree(hTpl);
		return NULL;
	}

	tpl = (DLGTEMPLATE FAR *)base;
	tpl->style = DS_MODALFRAME | WS_POPUPWINDOW | WS_DLGFRAME;
	tpl->cdit  = 11;
	tpl->x     = 20;
	tpl->y     = 20;
	tpl->cx    = 220;
	tpl->cy    = 158;

	p = base + sizeof(DLGTEMPLATE);
	p = DlgTplPutWord(p, 0x0000);		/* no menu */
	p = DlgTplPutWord(p, 0x0000);		/* default dialog class */
	p = DlgTplPutStr(p, "About %s");	/* matches CAPTION in the .rc */

	p = DlgTplAddItem(p, 184, 5, 40, 14, SAB_OKAY,
	                  WS_CHILD | WS_VISIBLE | WS_TABSTOP | WS_GROUP |
	                  BS_DEFPUSHBUTTON,
	                  0x0080 /* BUTTON */, "OK");

	p = DlgTplAddItem(p, 10, 3, 18, 16, SAB_ICON,
	                  WS_CHILD | WS_VISIBLE | SS_ICON,
	                  0x0082 /* STATIC */, "");

	p = DlgTplAddItem(p, 45, 5, 127, 10, SAB_ABOUT,
	                  WS_CHILD | WS_VISIBLE | SS_LEFT,
	                  0x0082, "osFree Janus %s");

	p = DlgTplAddItem(p, 45, 15, 100, 10, SAB_VERSION,
	                  WS_CHILD | WS_VISIBLE | SS_LEFT,
	                  0x0082, "");

	p = DlgTplAddItem(p, 45, 25, 145, 10, (WORD)0xFFFF,
	                  WS_CHILD | WS_VISIBLE | SS_LEFT,
	                  0x0082, "Copyright (c) 2023 osFree");

	p = DlgTplAddItem(p, 45, 35, 145, 20, SAB_TEXT,
	                  WS_CHILD | WS_VISIBLE | SS_LEFT,
	                  0x0082, "");

	p = DlgTplAddItem(p, 45, 85, 172, 1, (WORD)0xFFFF,
	                  WS_CHILD | WS_VISIBLE | SS_BLACKRECT,
	                  0x0082, "");

	p = DlgTplAddItem(p, 46, 55, 180, 20, SAB_USER,
	                  WS_CHILD | WS_VISIBLE | SS_LEFT,
	                  0x0082, "Static");

	p = DlgTplAddItem(p, 46, 65, 150, 10, SAB_WINDOW,
	                  WS_CHILD | WS_VISIBLE | SS_LEFT,
	                  0x0082, "Static");

	p = DlgTplAddItem(p, 46, 75, 150, 10, SAB_HOST,
	                  WS_CHILD | WS_VISIBLE | SS_LEFT,
	                  0x0082, "Host:");

	p = DlgTplAddItem(p, 46, 85, 150, 10, SAB_TERM,
	                  WS_CHILD | WS_VISIBLE | SS_LEFT,
	                  0x0082, "Display:");

	GlobalUnlock(hTpl);
	return hTpl;
}

#endif /* WINVER < 0x030A */

/*************************************************************************
 * ShellAbout	[SHELL.22]
 */

/*! @brief Display the standard "About" dialog.

    @param[in] hWnd          Parent window handle.
    @param[in] lpszCaption   Caption template. If it contains '#', the
                             part BEFORE '#' becomes the window caption
                             and the part AFTER '#' becomes the first
                             line of the dialog body. May be NULL.
    @param[in] lpszAboutText Additional text shown in the dialog body.
                             May be NULL.
    @param[in] hIcon         Optional icon to override the default;
                             NULL keeps the default.
*/
void WINAPI ShellAbout(HWND hWnd, LPCSTR lpszCaption, LPCSTR lpszAboutText, HICON hIcon)
{
	DLGPROC	lpProc;
	SHELLABOUTDATA sad;
	LPSHELLABOUTDATA lpsad;
	
	lpProc = MakeProcInstance((FARPROC)AboutDlgProc, Globals.hInstance);
	/* Source: Microsoft Windows 3.1 SDK, MakeProcInstance():
	   "If the function fails, the return value is NULL." A NULL
	   DLGPROC must not be passed to DialogBoxParam. */
	if (!lpProc)
		return;

	/* Source: MSDN ShellAbout() lists lpszCaption/lpszAboutText as
	   LPCSTR with no explicit "must not be NULL" restriction; treat
	   NULL as empty string rather than crashing. */
	sad.lpszText    = (LPSTR)(lpszAboutText ? lpszAboutText : "");
	sad.lpszCaption = (LPSTR)(lpszCaption   ? lpszCaption   : "");
	sad.hIcon       = hIcon;

	lpsad = &sad;

#if WINVER >= 0x030A
	DialogBoxParam(
		Globals.hInstance,
		MAKEINTRESOURCE(IDD_SHELLABOUT),
		hWnd,
		lpProc,
		(LPARAM)lpsad
		);
#else
	/* No resource in shell.lib: build the template in a global
	   block, run the dialog, then free the block. Win16
	   DialogBoxIndirectParam expects an HGLOBAL. */
	{
		HGLOBAL hTpl = BuildAboutTemplate();
		if (hTpl) {
			DialogBoxIndirectParam(
				Globals.hInstance,
				hTpl,
				hWnd,
				lpProc,
				(LPARAM)lpsad
				);
			GlobalFree(hTpl);
		}
	}
#endif

	FreeProcInstance(lpProc); 
}

/* ------------------------------------------------------------------ */
/* Dialog procedure                                                   */
/* ------------------------------------------------------------------ */

/*! @brief Dialog procedure for the Shell About dialog.

    @param[in] hDlg   Dialog window handle.
    @param[in] msg    Message identifier.
    @param[in] wParam Message parameter.
    @param[in] lParam Message parameter (points to SHELLABOUTDATA on
                      WM_INITDIALOG).

    @return TRUE when the message was handled, FALSE otherwise.

    @retval TRUE  The message was handled.
    @retval FALSE The message was not handled.
*/
BOOL CALLBACK AboutDlgProc(HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	LPSHELLABOUTDATA sad;
	char		abouttext[256];
	char		buf[256];
	char		caption[256];
	char		firstline[256];
	char		dirname[256];
	LPSTR		lpszTmp;
	DWORD		version;

	switch(msg) {
	case WM_INITDIALOG:
		sad = (LPSHELLABOUTDATA) lParam;

		/************************************************/
		/*	Caption first				*/
		/************************************************/
		/* '#' splits the caption: before '#' is the window
		   title, after '#' is the first line of the body.
		   lpszCaption is LPCSTR; copy into a local buffer. */
		lstrcpyn(caption, sad->lpszCaption, sizeof(caption));
		lpszTmp = lstrchr(caption, '#');
		if (lpszTmp) {
			*lpszTmp = '\0';
			SetWindowText(hDlg, caption);
			lstrcpyn(firstline, lpszTmp + 1, sizeof(firstline));
		} else {
			/* Read the dialog title from the template (Win 3.1:
			   CAPTION in the .rc; Win 3.0: the programmatic
			   template) and substitute the caption. */
			GetWindowText(hDlg, buf, sizeof(buf));
			wsprintf(abouttext, buf, caption);
			SetWindowText(hDlg, abouttext);
			lstrcpyn(firstline, caption, sizeof(firstline));
		}

		/************************************************/
		/*	Default first two lines			*/
		/************************************************/
		GetDlgItemText(hDlg, SAB_ABOUT, buf, sizeof(buf));
		wsprintf(abouttext, buf, firstline);
		SetDlgItemText(hDlg, SAB_ABOUT, abouttext);

		SetDlgItemText(hDlg, SAB_TEXT, sad->lpszText);

		/************************************************/
		/*	Twin specific 5 lines			*/
		/************************************************/
		GetModuleFileName(0, abouttext, 256);

		/* add any shell about specific string */
		SetDlgItemText(hDlg, SAB_USER, abouttext);

		GetWindowsDirectory(dirname, 256);
		wsprintf(abouttext, "Windows: %s", dirname);
		SetDlgItemText(hDlg, SAB_WINDOW, abouttext);

		GetSystemDirectory(dirname, 256);
		wsprintf(abouttext, "System: %s", dirname);
		SetDlgItemText(hDlg, SAB_SYSTEM, abouttext);

		/************************************************/
		/*	Host specific 2 lines			*/
		/************************************************/
		/* host system information */
		wsprintf(abouttext, "Host: DOS");
		SetDlgItemText(hDlg, SAB_HOST, abouttext);

		version = GetVersion();
#if WINVER >= 0x030A
		LoadString(Globals.hInstance, IDS_VERSION, buf, sizeof(buf));
#else
		/* No resources in shell.lib; hardcode the template. */
		lstrcpy(buf, "Version %s %s");
#endif
		{
			char szMajor[4];
			char szMinor[4];
			wsprintf(szMajor, "%d", LOBYTE(LOWORD(version)));
			wsprintf(szMinor, "%d", HIBYTE(LOWORD(version)));
			wsprintf(abouttext, buf, szMajor, szMinor);
		}

		SetDlgItemText(hDlg, SAB_VERSION, abouttext);

		/************************************************/
		/*	Icon passed in?				*/
		/************************************************/
		/* Win 3.1: SAB_ICON already shows the resource icon
		   ("willows.ico" from the .rc). Win 3.0: the control starts
		   empty, so set the system default icon via STM_SETICON.
		   The union avoids a pointer-to-integer warning when HICON
		   is declared as a far pointer by the Win16 headers. */
		if (sad->hIcon)
			SendDlgItemMessage(hDlg, SAB_ICON, STM_SETICON,
				sad->hIcon, 0L);
#if WINVER < 0x030A
		else {
			union { HICON h; WPARAM w; } u;
			u.h = LoadIcon((HINSTANCE)NULL, IDI_APPLICATION);
			SendDlgItemMessage(hDlg, SAB_ICON, STM_SETICON, u.w, 0L);
		}
#endif

		return TRUE;

	case WM_COMMAND:
		switch ( GET_WM_COMMAND_ID(wParam, lParam) )
		{
			case IDOK:
				EndDialog(hDlg, IDOK);
				break;
			case IDCANCEL:
				/* Esc key. */
				EndDialog(hDlg, IDCANCEL);
				break;
			default:
				return ( FALSE );
		}
		return ( TRUE );

	case WM_CLOSE:
		/* Title-bar close button and Alt+F4. */
		EndDialog(hDlg, IDCANCEL);
		return TRUE;
	}
	return FALSE;
}
