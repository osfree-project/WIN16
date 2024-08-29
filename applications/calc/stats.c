/*
 * WineCalc (stats.c)
 *
 * Copyright 2003 James Briggs
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
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#include <stdio.h> // sprintf
#include <string.h> // strlen

#include <windows.h>

#include "stats.h"
#include "resource.h"
#include "winecalc.h"

    HWND hWndListBox;

    extern CALC calc;
    extern HWND hWndDlgStats;

BOOL CALLBACK StatsDlgProc( HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
    HDC hdc;
    PAINTSTRUCT ps;


    switch( uMsg ) {

    case WM_INITDIALOG:
        hWndListBox = CreateWindow(
            "LISTBOX",	// pointer to registered class name
            "Listbox",	// pointer to window name
            WS_CHILD | WS_VISIBLE | WS_VSCROLL | WS_BORDER | LBS_NOINTEGRALHEIGHT,	// window style
                    6,	// horizontal position of window
                    6,	// vertical position of window
                  208,	// window width
                   66,	// window height
                 hDlg,	// handle to parent or owner window
                 0/*NULL*/,	// handle to menu or child-window identifier
                 0/*NULL*/,	// handle to application instance
                 NULL 	// pointer to window-creation data
        );

		ShowWindow(hWndListBox, SW_SHOW);

		SendMessage (hWndListBox, WM_SETFONT, (UINT)GetStockObject(SYSTEM_FONT), TRUE);

		// SetFocus(hWndDlgStats);

        return TRUE;

    case WM_COMMAND:
        switch( LOWORD( wParam ) ) {

        case ID_STATS_RET:
			SetFocus(GetParent(hDlg));
			return 0;

		case ID_STATS_LOAD:
			{
			   int i;

			   i = SendMessage(hWndListBox, LB_GETCURSEL, 0, 0);
                           SendMessage(hWndListBox, LB_GETTEXT,   i, (LPARAM)calc.buffer);
			   calc_buffer_display(&calc);
                        }
            return 0;

		case ID_STATS_CD:
			{
			   int i;

			   i = SendMessage(hWndListBox, LB_GETCURSEL, 0, 0);
                           SendMessage(hWndListBox, LB_DELETESTRING, i, 0);
                           InvalidateRect(hDlg,NULL,TRUE);
			   UpdateWindow(hDlg);
                        }
			return 0;

		case ID_STATS_CAD:
			SendMessage(hWndListBox, LB_RESETCONTENT, 0, 0);
			InvalidateRect(hDlg,NULL,TRUE);
	        UpdateWindow(hDlg);
			return 0;
        }
        break;

    case WM_PAINT:
      {
        char s[CALC_BUF_SIZE];
        int lb_count;
        HFONT hFont;
        HFONT hFontOrg;

        hdc = BeginPaint( hDlg, &ps );
        hFont = GetStockObject(SYSTEM_FONT);
        hFontOrg = SelectObject(hdc, hFont);

        lb_count = SendMessage(hWndListBox, LB_GETCOUNT, 0, 0);
        sprintf(s, "n=%d", lb_count);

        SetBkMode(hdc, TRANSPARENT);
        TextOut(hdc, 98, 121, s, strlen(s));
        SelectObject(hdc, hFontOrg);
        EndPaint( hDlg, &ps );

        return 0;
      }
	case WM_CLOSE:
        hWndDlgStats = 0;                                 // invalidate stats dialog
        SendMessage(GetParent(hDlg), WM_CHAR, '\x13', 1); // disable stats related calculator buttons
        DestroyWindow( hDlg );

        return 0;
    }
    return FALSE;
}

