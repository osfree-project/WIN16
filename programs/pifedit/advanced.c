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
	switch (msg) {

	case WM_CLOSE:
		{
			EndDialog(hWnd, IDCANCEL);
		}
		break;


	case WM_INITDIALOG:
		{
			// Copy the data to the dialog controls from our .PIF model
			char szNumBuf[32];
			int 	n;

			PPIFHDR	pPIFHDR = (PPIFHDR) &Globals.pifModel;
			PPIFSIG	pPifSig = NULL;
		//  PPIF286	pPif286 = NULL;
			PPIF386	pPif386 = NULL;

			// Verify the checksum
			if (pPIFHDR->CheckSum != 0 && pPIFHDR->CheckSum != ComputePIFChecksum(&Globals.pifModel)) 
			{
				char	szBuf[80];

				LoadString(Globals.hInst, IDS_BADCHECKSUM, szBuf, sizeof(szBuf));
				MessageBox(hWnd, szBuf, Globals.szAppTitle, MB_OK);
			}

			// Find the 386 Enhanced mode section
			pPifSig = (PPIFSIG) (pPIFHDR + 1);

			while ( ((n = lstrcmp(pPifSig->Signature, "WINDOWS 386 3.0")) != 0) &&
				pPifSig->NextOff != 0xFFFF &&
				pPifSig->NextOff > 0 &&
				(PBYTE) pPifSig < (PBYTE) (&Globals.pifModel+1) ) {

				pPifSig = (PPIFSIG) ((PBYTE) pPIFHDR + pPifSig->NextOff);

			}

			if (n) {	// No 386 Enhanced section -- use the defaults
				char	szBuf[80];

				LoadString(Globals.hInst, IDS_BADCHECKSUM, szBuf, sizeof(szBuf));
				MessageBox(hWnd, szBuf, Globals.szAppTitle, MB_OK);
				return (1);
			}

			pPif386 = (PPIF386) ((PBYTE) pPIFHDR + pPifSig->DataOff);

			// Fill in the controls

			CheckDlgButton(hWnd, IDB_DOSLOCK, (pPif386->TaskFlags & 0x0400) ? 1 : 0);
			CheckDlgButton(hWnd, IDB_EMSLOCK, (pPif386->TaskFlags & 0x0080) ? 1 : 0);
			CheckDlgButton(hWnd, IDB_XMSLOCK, (pPif386->TaskFlags & 0x0100) ? 1 : 0);
			CheckDlgButton(hWnd, IDB_XMSHMA, (!(pPif386->TaskFlags & 0x0020)) ? 1 : 0);

			wsprintf(szNumBuf,"%d",pPif386->ForePrio);
			SetDlgItemText(hWnd, IDE_FOREPRIO, szNumBuf);

			wsprintf(szNumBuf,"%d",pPif386->BackPrio);
			SetDlgItemText(hWnd, IDE_BACKPRIO, szNumBuf);

			CheckDlgButton(hWnd, IDB_DETECTIDLE, pPif386->TaskFlags & 0x0010 ? 1 : 0);


			CheckDlgButton(hWnd, IDB_FASTPASTE,  pPif386->TaskFlags & 0x0200 ? 1 : 0);
			CheckDlgButton(hWnd, IDB_ALLOWCLOSE, pPif386->WinFlags & 0x01 ? 1 : 0);

			CheckDlgButton(hWnd, IDB_MONTEXT, (!(pPif386->VidFlags & 0x02)) ? 1 : 0);
			CheckDlgButton(hWnd, IDB_MONLOW,  (!(pPif386->VidFlags & 0x04)) ? 1 : 0);
			CheckDlgButton(hWnd, IDB_MONHIGH, (!(pPif386->VidFlags & 0x08)) ? 1 : 0);

			CheckDlgButton(hWnd, IDB_EMULATE, pPif386->VidFlags & 0x01 ? 1 : 0);
			CheckDlgButton(hWnd, IDB_RETAIN,  pPif386->VidFlags & 0x80 ? 1 : 0);

			CheckDlgButton(hWnd, IDB_ALTENTER, pPif386->TaskFlags & 0x0001 ? 1 : 0);
			CheckDlgButton(hWnd, IDB_ALTPRTSC, pPif386->TaskFlags & 0x0002 ? 1 : 0);
			CheckDlgButton(hWnd, IDB_PRTSC, pPif386->TaskFlags & 0x0004 ? 1 : 0);
			CheckDlgButton(hWnd, IDB_CTRLESC, pPif386->TaskFlags & 0x0008 ? 1 : 0);

			CheckDlgButton(hWnd, IDB_ALTTAB, pPif386->WinFlags & 0x20 ? 1 : 0);
			CheckDlgButton(hWnd, IDB_ALTESC, pPif386->WinFlags & 0x40 ? 1 : 0);
			CheckDlgButton(hWnd, IDB_ALTSPACE, pPif386->WinFlags & 0x80 ? 1 : 0);

			if (pPif386->TaskFlags & 0x0040) {	// User-definable hotkey
				Globals.wHotkeyScancode = pPif386->Hotkey;
				Globals.bHotkeyBits = pPif386->HotkeyBits;

				CheckDlgButton(hWnd, IDB_ALT, pPif386->HotkeyShift & 0x08 ? 1 : 0);
				CheckDlgButton(hWnd, IDB_CTRL, pPif386->HotkeyShift & 0x04 ? 1 : 0);
				CheckDlgButton(hWnd, IDB_SHIFT, pPif386->HotkeyShift & 0x03 ? 1 : 0);

				GetKeyNameText(
					MAKELPARAM(1, pPif386->Hotkey |
						((WORD) (pPif386->HotkeyBits & 1) << 8) |
						0x0200
						),
					szNumBuf,
					sizeof(szNumBuf)
					);
				SetDlgItemText(hWnd, IDE_KEY, szNumBuf);

			} else {
				Globals.wHotkeyScancode = 0;
				Globals.bHotkeyBits = 0;

				CheckDlgButton(hWnd, IDB_ALT, 0);
				CheckDlgButton(hWnd, IDB_CTRL, 0);
				CheckDlgButton(hWnd, IDB_SHIFT, 0);
				SetDlgItemText(hWnd, IDE_KEY, NONE_STR);

			}

		}

	}

	return (FALSE);
}
