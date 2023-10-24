#include "main.h"

int MessageBoxString(HWND hWnd, HINSTANCE hInst, UINT uID, UINT uType)
{
	char	szBuf[128];

	LoadString(hInst, uID, szBuf, sizeof(szBuf));
	return MessageBox(hWnd, szBuf, Globals.szAppTitle, uType);
}

void FAR cdecl MessageBoxPrintf(LPSTR szFormat, ...)
{
    char ach[256];
    int  s,d;

    s = wvsprintf(ach, szFormat, (LPSTR) (&szFormat + 1));

    for (d = sizeof(ach) - 1; s >= 0; s--) {
	if ((ach[d--] = ach[s]) == '\n')
	    ach[d--] = '\r';
    }

    MessageBox(0, ach+d+1, "MessageBoxPrintf()", MB_OK);
}
