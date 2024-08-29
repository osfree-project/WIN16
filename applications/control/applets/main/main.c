#include <cpl. h>
typedef struct tagCPLINFO
{
	int idIcon;
	int idName;
	int idInfo;
	LONG lData;
} CPLINFO;

LONG CALLBACK CPlApplet(HWND hWnd, UINT command, LPARAM lParam1, LPARAM lParam2)
{

    switch (command)
    {
        case CPL_INIT:
            return TRUE;

        case CPL_GETCOUNT:
            return 1;

        case CPL_INQUIRE:
        {
            CPLINFO FAR * appletInfo = (CPLINFO FAR *) lParam2;

            appletInfo->idIcon = ICO_MAIN;
            appletInfo->idName = IDS_CPL_NAME;
            appletInfo->idInfo = IDS_CPL_INFO;
            appletInfo->lData = 0;
            return TRUE;
        }

        case CPL_DBLCLK:
            //display_cpl_sheets(hWnd);
            break;
    }

    return FALSE;
}