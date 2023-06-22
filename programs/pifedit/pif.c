/************************  The Qualitas PIF Editor  ***************************
 *									      *
 *	     (C) Copyright 1992, 1993 Qualitas, Inc.  GNU General Public License version 3.    *
 *									      *
 *  MODULE   :	PIF.C - PIF parser
 *									      *
 ******************************************************************************/

#define NOCOMM			// Avoid inclusion of bulky COMM driver stuff
#include <windows.h>

#include <string.h>		// For memset

#include "qpifedit.h"

extern PIF	pifModel;	// Complete 545-byte .PIF image
extern PIF	pifBackup;	// Complete 545-byte .PIF image before editing

/****************************************************************************
 *
 *  FUNCTION :	ComputePIFChecksum(PPIF)
 *
 *  PURPOSE  :	Calculate the checksum on the old-style .PIF header
 *
 *  ENTRY    :	PPIF	pPIF;		// ==> .PIF structure
 *
 *  RETURNS  :	Checksum as a BYTE
 *
 ****************************************************************************/

BYTE ComputePIFChecksum(PPIF pPIF)		// Checksum the PIFHDR
{
    PBYTE	pb = (PBYTE) pPIF;
    PBYTE	pbz = pb + sizeof(PIFHDR);	// Size of old-style header
    BYTE	bSum;

    pb += 2;		// Skip over first unknown byte and checksum

    for (bSum = 0; pb < pbz; pb++) bSum += *pb;

    return (bSum);
}

/****************************************************************************
 *
 *  FUNCTION :	DefaultPIF(VOID)
 *
 *  PURPOSE  :	Fill in the .PIF structure with the standard default values
 *
 *  ENTRY    :	VOID
 *
 *  RETURNS  :	VOID
 *
 ****************************************************************************/

VOID DefaultPIF(VOID)
{
    memset(&pifModel, 0, sizeof(PIF));

    lstrcpy(pifModel.pifSigEX.Signature, "MICROSOFT PIFEX");
    pifModel.pifSigEX.DataOff = 0;					// 0000
    pifModel.pifSigEX.DataLen = sizeof(PIFHDR); 			// 0171
    pifModel.pifSigEX.NextOff = sizeof(PIFHDR) + sizeof(PIFSIG);	// 0187

    lstrcpy(pifModel.pifSig286.Signature, "WINDOWS 286 3.0");
    pifModel.pifSig286.DataOff = pifModel.pifSigEX.NextOff + sizeof(PIFSIG);
    pifModel.pifSig286.DataLen = sizeof(PIF286);
    pifModel.pifSig286.NextOff = sizeof(PIFHDR) + 2*sizeof(PIFSIG)+ sizeof(PIF286);

    lstrcpy(pifModel.pifSig386.Signature, "WINDOWS 386 3.0");
    pifModel.pifSig386.DataOff = pifModel.pifSig286.NextOff + sizeof(PIFSIG);
    pifModel.pifSig386.DataLen = sizeof(PIF386);
    pifModel.pifSig386.NextOff = 0xFFFF;

    pifModel.pifHdr.Title[0] = '\0';
    pifModel.pifHdr.PgmName[0] = '\0';
    pifModel.pifHdr.StartupDir[0] = '\0';
    pifModel.pifHdr.CmdLineS[0] = '\0';

// TopView fields
    pifModel.pifHdr.ScreenType = 0x7F;		// Windows doesn't use this
    pifModel.pifHdr.ScreenPages = 1;		// # of screen pages
    pifModel.pifHdr.IntVecLow = 0;		// Low	bound of INT vectors
    pifModel.pifHdr.IntVecHigh = 255;		// High bound of INT vectors

    pifModel.pifHdr.ScrnRows = 25;		// Windows doesn't use this
    pifModel.pifHdr.ScrnCols = 80;		// Windows doesn't use this
//  pifModel.pifHdr.RowOffs = 80;		// Windows doesn't use this
//  pifModel.pifHdr.ColOffs = 80;		// Windows doesn't use this

    pifModel.pifHdr.SystemMem = 7;		// 7 - text, 23 - graphics

    pifModel.pifHdr.DosMaxS = 640;
    pifModel.pifHdr.DosMinS = 128;
    pifModel.pifHdr.Flags1 = 0x10;		// Close window on exit

    pifModel.pif386.DosMax = 640;
    pifModel.pif386.DosMin = 128;

    pifModel.pif386.ForePrio = 100;
    pifModel.pif386.BackPrio = 50;

    pifModel.pif386.EmsMax = 1024;
    pifModel.pif386.EmsMin = 0;
    pifModel.pif386.XmsMax = 1024;
    pifModel.pif386.XmsMin = 0;

    pifModel.pif386.WinFlags = 0x08;		// Fullscreen
    pifModel.pif386.TaskFlags = 0x0200 | 0x0010; // Allow faste paste
						// Detect idle time
    pifModel.pif386.VidFlags = 0x1F;		// Video mode text
						// Emulate text mode
						// No monitor ports
    pifModel.pif386.CmdLine3[0] = '\0';

    pifModel.pifHdr.CheckSum = ComputePIFChecksum(&pifModel);

}

/****************************************************************************
 *
 *  FUNCTION :	SnapPif(VOID)
 *
 *  PURPOSE  :	Copy the .PIF model to a backup for CheckSave()
 *		inspection later
 *
 *  ENTRY    :	VOID
 *
 *  RETURNS  :	VOID
 *
 ****************************************************************************/

VOID SnapPif(VOID)
{
    memcpy(&pifBackup, &pifModel, sizeof(PIF));
}

/****************************************************************************
 *
 *  FUNCTION :	WritePIF(LPSTR, LPSTR)
 *
 *  PURPOSE  :	Helper function for MenuFileSave(), etc.
 *		Does the dirty work of the COMMDLG GetSaveileName()
 *		Writes the .PIF file from local memory
 *
 *  ENTRY    :	LPSTR	lpszPIFName;	// ==> .PIF filespec
 *
 *  RETURNS  :	0	- Normal return
 *		IDABORT - File system error
 *
 ****************************************************************************/

int WritePIF(HWND hWndDlg, LPSTR lpszPIFName)
{
    register int	rc;
    HCURSOR	hCursorOld;
    OFSTRUCT	OpenBuff;

    HFILE	hFilePIF = 0;

/* Switch to the hour glass cursor */

    SetCapture(hWndDlg);
    hCursorOld = SetCursor(LoadCursor(NULL, IDC_WAIT));

/* Open and write the file */

    DebugPrintf("SavePIF(): OpenFile(%s, OF_READ)\r\n", lpszPIFName);

    hFilePIF = OpenFile(lpszPIFName, &OpenBuff, OF_CREATE | OF_WRITE);

    if ((int) hFilePIF != HFILE_ERROR) {

	ControlsToPIF(hWndDlg);

	rc = _lwrite(hFilePIF, (LPVOID) (PPIF) &pifModel, sizeof(PIF));
	if (rc != sizeof(PIF)) {
	    DebugPrintf("SavePIF() _lwrite(hFilePIF, (LPVOID) (PPIF) &pifModel, sizeof(PIF)) failed %d\r\n", rc);
	    rc = IDABORT;  goto WRITEPIF_EXIT;
	}
    } else {	/* Couldn't save the file */
		DebugPrintf("SavePIF() OpenFile(%s, &OpenBuff, OF_CREATE | OF_WRITE) failed\r\n", lpszPIFName);
		rc = IDABORT;  goto WRITEPIF_EXIT;

    }

    rc = 0;

WRITEPIF_EXIT:
    SetCursor(hCursorOld);
    ReleaseCapture();

    if (hFilePIF > 0) _lclose(hFilePIF);

    return (rc);
}
