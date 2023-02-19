#include <windows.h>

/*************************************************************
 *      GetKbdTable()    [KEYBOARD.1]
 *
 *	parmW:	Keyboard type	1: XT, M24 83-key
 *				2: Olivetti M24 102-key 'ICO'
 *				3: AT 84- or 86-key
 *				4: RT Enhanced 101- or 102-key
 *				Old Ericsson keyboards:
 *				5: Nokia (aka Ericsson) 1050
 *				6: Nokia
 *
 *	parmD:	far Pointer to keyTrTab[] in driver
 *
 *	parmD:	far Pointer to the header for the translation tables
 *
 *	The tables are patched, and various data are copied to
 *	the driver.
 *
 *	This returns a FAR pointer to this DLL's DATA segment in DX:AX
 *	(AX is 0).
 *
 *	This routine is in a LOADONCALL DISPOSABLE segment, so
 *	its memory may be reclaimed.  The DATA segment is fixed.
 *
 */

LPVOID WINAPI GetKbdTable(WORD wKeyType, LPBYTE leyTrTab, LPBYTE xlat)
{
  return NULL;
}

/*************************************************************
 *      GetKeyString()    [KEYBOARD.2]
 *
 *	nString		index to list of strings
 *
 *	lpStringOut	selected string is copied to this address
 *
 *	The size of the string (exclusive of NULL termination) is
 *	returned in AX.
 */

int WINAPI GetKeyString(int nString, LPSTR lpStringOut)
{
  return NULL;
}

BOOL WINAPI LibMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved)
{

    return TRUE;
}
