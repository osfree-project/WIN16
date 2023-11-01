/*
 * Windows Task Manager
 *
 * Copyright 2023 by Yuri Prokushev
 *
 * osFree licence.
 */

/*
  Heavely based on Undocumented Windows
*/

#include <stdio.h>	// sscanf

#include "taskman.h"

/* Globals */
static WORD nLeft=0, nTop=0;

BOOL CALLBACK EnumWindowsProc(HWND   hWnd, LPARAM lParam)
{
	char szBuf[80];

	if (IsWindowVisible(hWnd)) 
	{
		GetWindowText(hWnd, szBuf, sizeof(szBuf));
		if (lstrlen(szBuf))
		{
			SendMessage((HWND)lParam, LB_SETITEMDATA, SendMessage((HWND)lParam, LB_ADDSTRING, 0, (LPARAM)(LPCSTR)szBuf), (LPARAM)hWnd);
		}
	}
  
	return TRUE;
}

/****************************************************************************
 *
 *  FUNCTION :	TaskManDlgProcc(HWND, UINT, WPARAM, LPARAM)
 *
 *  PURPOSE  :	Dialog proc for the frame
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

BOOL CALLBACK _export TaskManDlgProc(HWND hWndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg) {
	case WM_INITDIALOG:
		{
			HWND hListBox=GetDlgItem(hWndDlg, IDD_TASKLIST);

			SendMessage(hListBox, WM_SETREDRAW, FALSE, 0);
			EnumWindows(EnumWindowsProc, (LPARAM)hListBox);
			SendMessage(hListBox, LB_SETCURSEL, 0, 0);
			SendMessage(hListBox, WM_SETREDRAW, TRUE, 0);
			InvalidateRect(hListBox, NULL, TRUE);
			return FALSE;
		}

	case WM_CLOSE:
		EndDialog(hWndDlg, IDCANCEL);
		return FALSE;
		break;

	case WM_COMMAND:
		switch (wParam) {
			case IDD_TASKLIST:
			{
				if(HIWORD(lParam) != LBN_DBLCLK)
			        {
					break;
				}
				// passthru
			}
			case IDD_SWITCHTO:
			{
				HWND hwnd, hwndPopup;
				HWND hListBox=GetDlgItem(hWndDlg, IDD_TASKLIST);
	
				ShowWindow(hWndDlg, SW_HIDE);
	
				if (IsWindow(hwnd=SendMessage(hListBox, LB_GETITEMDATA, SendMessage(hListBox, LB_GETCURSEL, 0, 0), 0)))
				{
					if (IsWindow(hwndPopup=GetLastActivePopup(hwnd)))
					{
						if (!(GetWindowLong(hwndPopup, GWL_STYLE) & WS_DISABLED))
						{
							SwitchToThisWindow(hwndPopup, TRUE);
						}
					}
				}
				EndDialog(hWndDlg, IDCANCEL);
				break;
			}
			case IDD_ENDTASK:
			{
				HWND hwnd, hwndPopup;
				HWND hListBox=GetDlgItem(hWndDlg, IDD_TASKLIST);

				ShowWindow(hWndDlg, SW_HIDE);

				if (IsWindow(hwnd=SendMessage(hListBox, LB_GETITEMDATA, SendMessage(hListBox, LB_GETCURSEL, 0, 0), 0)))
				{
					if (IsWindow(hwndPopup=GetLastActivePopup(hwnd)))
					{
						if (!(GetWindowLong(hwndPopup, GWL_STYLE) & WS_DISABLED))
						{
							SwitchToThisWindow(hwndPopup, TRUE);
							PostMessage(hwndPopup, WM_CLOSE, 0, 0);
						}
					}
				}
				EndDialog(hWndDlg, IDCANCEL);
				break;
			}
			case IDCANCEL:
			{
				ShowWindow(hWndDlg, SW_HIDE);
				EndDialog(hWndDlg, IDCANCEL);
				break;
			}
			case IDD_CASCADE:
			{
				ShowWindow(hWndDlg, SW_HIDE);
				CascadeChildWindows(GetDesktopWindow(), 0);
				EndDialog(hWndDlg, IDCANCEL);
				break;
			}
			case IDD_TILE:
			{
				HWND hWndDesktop;
				ShowWindow(hWndDlg, SW_HIDE);
				hWndDesktop=GetDesktopWindow();
				if (GetKeyState(VK_SHIFT) == 0x8000)
					TileChildWindows(hWndDesktop, MDITILE_HORIZONTAL);
				else
					TileChildWindows(hWndDesktop, MDITILE_VERTICAL);
				EndDialog(hWndDlg, IDCANCEL);
				break;
			}
			case IDD_ARRANGEICONS:
			{
				ShowWindow(hWndDlg, SW_HIDE);
				ArrangeIconicWindows(GetDesktopWindow());
				EndDialog(hWndDlg, IDCANCEL);
				break;
			}
		        break;
		}

	    default:
	        return FALSE;

	}

	return TRUE;
}

int PASCAL WinMain (HINSTANCE inst, HINSTANCE prev, LPSTR cmdline, int show)
{
	VOID (FAR PASCAL * RegisterPenApp)(UINT, BOOL);
	FARPROC fpDlgProc;
	char CmdLine[260];

	if (prev) 
	{
		return 0;
	}

	if (*cmdline)
	{
		lstrcpy(CmdLine, cmdline);
		sscanf(CmdLine, "%u %u", &nLeft, &nTop);
	}

	RegisterPenApp=GetProcAddress(GetSystemMetrics(SM_PENWINDOWS), "RegisterPenApp");

	if (RegisterPenApp)
		(*RegisterPenApp)(RPA_DEFAULT, TRUE);

	if (fpDlgProc=MakeProcInstance(TaskManDlgProc, inst))
	{
		DialogBox(inst, MAKEINTRESOURCE(10), 0, fpDlgProc);
		FreeProcInstance(fpDlgProc);
	}

	if (RegisterPenApp)
	{
		(*RegisterPenApp)(RPA_DEFAULT, FALSE);
	}

	return 0;

}
