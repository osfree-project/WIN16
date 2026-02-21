/*    
	SysMetrics.c	2.32
    	Copyright 1997 Willows Software, Inc. 

This library is free software; you can redistribute it and/or
modify it under the terms of the GNU Lesser General Public
License as published by the Free Software Foundation; either
version 2.1 of the License, or (at your option) any later version.

This library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public
License along with this library; if not, see
<https://www.gnu.org/licenses/>.

 */


#include <user.h>


/* SM_ARRANGE return codes */
#define ARW_BOTTOMLEFT		0x0000L
#define ARW_BOTTOMRIGHT		0x0001L
#define ARW_TOPLEFT		0x0002L
#define ARW_TOPRIGHT		0x0003L
#define ARW_STARTMASK		0x0003L
#define ARW_STARTRIGHT		0x0001L
#define ARW_STARTTOP		0x0002L

#define ARW_LEFT		0x0000L
#define ARW_RIGHT		0x0000L
#define ARW_UP			0x0004L
#define ARW_DOWN		0x0004L
#define ARW_HIDE		0x0008L
#define ARW_VALID		0x000FL




static BOOL bWarningBeep = FALSE;
static BOOL bFastTaskSwitch = FALSE;
static BOOL bIconTitleWrap = FALSE;
static BOOL bScreenSaveActive = FALSE;
static int nScreenSaveTimeOut = 0;
static int nGridGranularity = 0;
static int nKeyboardDelay = 0;
static int nKeyboardSpeed = 0;
static int nDoubleClickTime = 0;
static const LOGFONT lgfDefaultIconTitleFont =
  {	12,
	0,
	0,
	0,
	300,
	0,
	0,
	0,
	0,
	0,
	0,
	DEFAULT_QUALITY,
	DEFAULT_PITCH,
	"Helv"
  };

static LOGFONT lgfIconTitleFont = 
  {	12,
	0,
	0,
	0,
	300,
	0,
	0,
	0,
	0,
	0,
	0,
	DEFAULT_QUALITY,
	DEFAULT_PITCH,
	"Helv"
  };


#define SPI_SETWORKAREA             47
#define SPI_GETWORKAREA             48

/***********************************************************************
 *		GetSystemMetrics (USER.179)
 */
int WINAPI GetSystemMetrics(int nIndex)
{
	int retVal;

	PushDS();
	SetUserHeapDS();
	FUNCTION_START

	retVal=0;
	/* get system metrics value */
	if ((nIndex >= 0) && (nIndex < SM_CMETRICS))
	{
		retVal = SysMetricsDef[nIndex];
	}

	FUNCTION_END
	PopDS();
	return retVal;

}

/***********************************************************************
 *		SystemParametersInfo (USER.483)
 */
BOOL WINAPI SystemParametersInfo(UINT uAction, UINT uParam, LPVOID lpvParam,
			UINT fuWinIni)
{
    BOOL bSendWinIniChange = FALSE;
    char szBuffer[80];
    char lpszSection[80];
    LPRECT lpRect;

//    APISTR((LF_APICALL,
//	"SystemParametersInfo(UINT=%x,UINT=%x,LPVOID=%x,UINT=%x)\n",
//	uAction, uParam, lpvParam,fuWinIni));

    switch (uAction) {
	case SPI_GETBEEP:
	    *((BOOL *)lpvParam) = bWarningBeep;
//    	    APISTR((LF_APIRET,"SystemParametersInfo: returns BOOL TRUE\n"));
	    return TRUE;
	case SPI_SETBEEP:
	    bWarningBeep = (BOOL)uParam;
	    if (fuWinIni & SPIF_UPDATEINIFILE) {
		lstrcpy(lpszSection,"windows");
		WriteProfileString(lpszSection,"Beep",(BOOL)uParam?"yes":"no");
		if (fuWinIni & SPIF_SENDWININICHANGE)
		    bSendWinIniChange = TRUE;
	    }
	    break;

	case SPI_GETBORDER:
	case SPI_SETBORDER:
//    	    APISTR((LF_APIRET,"SystemParametersInfo: returns BOOL FALSE\n"));
	    return FALSE;

	case SPI_GETFASTTASKSWITCH:
	    *((BOOL *)lpvParam) = bFastTaskSwitch;
//    	    APISTR((LF_APIRET,"SystemParametersInfo: returns BOOL TRUE\n"));
	    return TRUE;
	case SPI_SETFASTTASKSWITCH:
	    bFastTaskSwitch = (BOOL)uParam;
	    if (fuWinIni & SPIF_UPDATEINIFILE) {
		lstrcpy(lpszSection,"windows");
		WriteProfileString(lpszSection,"CoolSwitch",
				(BOOL)uParam?"1":"0");
		if (fuWinIni & SPIF_SENDWININICHANGE)
		    bSendWinIniChange = TRUE;
	    }
	    break;

	case SPI_GETGRIDGRANULARITY:
	    *((LPINT)lpvParam) = nGridGranularity;
//    	    APISTR((LF_APIRET,"SystemParametersInfo: returns BOOL TRUE\n"));
	    return TRUE;

	case SPI_SETGRIDGRANULARITY:
	    nGridGranularity = (int)uParam;
	    if (fuWinIni & SPIF_UPDATEINIFILE) {
		lstrcpy(lpszSection,"desktop");
		lstrcpy(szBuffer, itoa(uParam));//sprintf(szBuffer,"%d",(int)uParam);
		WriteProfileString(lpszSection,"GridGranularity",szBuffer);
		if (fuWinIni & SPIF_SENDWININICHANGE)
		    bSendWinIniChange = TRUE;
	    }
	    break;

	case SPI_GETICONTITLELOGFONT:
	    _fmemcpy(lpvParam,(LPVOID)&lgfIconTitleFont,
			min(uParam,sizeof(LOGFONT)));
//    	    APISTR((LF_APIRET,"SystemParametersInfo: returns BOOL TRUE\n"));
	    return TRUE;

	case SPI_SETICONTITLELOGFONT:
	    if (!lpvParam) {
		if (!uParam) {
		    _fmemcpy((LPVOID)&lgfIconTitleFont,
			   (LPVOID)&lgfDefaultIconTitleFont,
			   sizeof(LOGFONT));
//    	    	    APISTR((LF_APIRET,"SystemParametersInfo: returns BOOL TRUE\n"));
		    return TRUE;
		}
		else {
//    	    	    APISTR((LF_APIRET,"SystemParametersInfo: returns BOOL FALSE\n"));
		    return FALSE;
		}
	    }
	    _fmemcpy((LPVOID)&lgfIconTitleFont,lpvParam,sizeof(LOGFONT));
//    	    APISTR((LF_APIRET,"SystemParametersInfo: returns BOOL TRUE\n"));
	    return TRUE;

	case SPI_GETICONTITLEWRAP:
	    *((BOOL *)lpvParam) = bIconTitleWrap;
//    	    APISTR((LF_APIRET,"SystemParametersInfo: returns BOOL TRUE\n"));
	    return TRUE;

	case SPI_SETICONTITLEWRAP:
	    bIconTitleWrap = (BOOL)uParam;
	    if (fuWinIni & SPIF_UPDATEINIFILE) {
		lstrcpy(lpszSection,"desktop");
		WriteProfileString(lpszSection,"IconTitleWrap",
			(BOOL)uParam?"1":"0");
		if (fuWinIni & SPIF_SENDWININICHANGE)
		    bSendWinIniChange = TRUE;
	    }
	    break;

	case SPI_GETKEYBOARDDELAY:
	    *((LPINT)lpvParam) = nKeyboardDelay;
//    	    APISTR((LF_APIRET,"SystemParametersInfo: returns BOOL TRUE\n"));
	    return TRUE;

	case SPI_SETKEYBOARDDELAY:
	    nKeyboardDelay = (int)uParam;
	    if (fuWinIni & SPIF_UPDATEINIFILE) {
		lstrcpy(lpszSection,"windows");
		lstrcpy(szBuffer, itoa(uParam));//sprintf(szBuffer,"%d",(int)uParam);
		WriteProfileString(lpszSection,"KeyboardDelay",szBuffer);
		if (fuWinIni & SPIF_SENDWININICHANGE)
		    bSendWinIniChange = TRUE;
	    }
	    break;

	case SPI_GETKEYBOARDSPEED:
	    *((LPINT)lpvParam) = nKeyboardSpeed;
//    	    APISTR((LF_APIRET,"SystemParametersInfo: returns BOOL TRUE\n"));
	    return TRUE;

	case SPI_SETKEYBOARDSPEED:
	    nKeyboardSpeed = (int)uParam;
	    if (fuWinIni & SPIF_UPDATEINIFILE) {
		lstrcpy(lpszSection,"windows");
		lstrcpy(szBuffer, itoa(uParam));//sprintf(szBuffer,"%d",(int)uParam);
		WriteProfileString(lpszSection,"KeyboardSpeed",szBuffer);
		if (fuWinIni & SPIF_SENDWININICHANGE)
		    bSendWinIniChange = TRUE;
	    }
	    break;

	case SPI_GETMENUDROPALIGNMENT:
	    *((LPINT)lpvParam) = GETSYSTEMMETRICS(SM_MENUDROPALIGNMENT);
//    	    APISTR((LF_APIRET,"SystemParametersInfo: returns BOOL TRUE\n"));
	    return TRUE;

	case SPI_SETMENUDROPALIGNMENT:
	    SysMetricsDef[SM_MENUDROPALIGNMENT] = (int)uParam;
//    	    APISTR((LF_APIRET,"SystemParametersInfo: returns BOOL TRUE\n"));
	    return TRUE;

	case SPI_GETMOUSE:
	case SPI_SETMOUSE:
//    	    APISTR((LF_APIRET,"SystemParametersInfo: returns BOOL FALSE\n"));
	    return FALSE;

	case SPI_GETSCREENSAVEACTIVE:
	    *((BOOL *)lpvParam) = bScreenSaveActive;
//    	    APISTR((LF_APIRET,"SystemParametersInfo: returns BOOL TRUE\n"));
	    return TRUE;

	case SPI_SETSCREENSAVEACTIVE:
	    bScreenSaveActive = (BOOL)uParam;
	    if (fuWinIni & SPIF_UPDATEINIFILE) {
		lstrcpy(lpszSection,"windows");
		WriteProfileString(lpszSection,"ScreenSaveActive",
			(BOOL)uParam?"1":"0");
		if (fuWinIni & SPIF_SENDWININICHANGE)
		    bSendWinIniChange = TRUE;
	    }
	    break;

	case SPI_GETSCREENSAVETIMEOUT:
	    *((BOOL *)lpvParam) = nScreenSaveTimeOut;
//    	    APISTR((LF_APIRET,"SystemParametersInfo: returns BOOL TRUE\n"));
	    return TRUE;

	case SPI_SETSCREENSAVETIMEOUT:
	    nScreenSaveTimeOut = (int)uParam;
	    if (fuWinIni & SPIF_UPDATEINIFILE) {
		lstrcpy(lpszSection,"windows");
		lstrcpy(szBuffer, itoa(nScreenSaveTimeOut));//sprintf(szBuffer,"%d",nScreenSaveTimeOut);
		WriteProfileString(lpszSection,"ScreenSaveTimeOut",szBuffer);
		if (fuWinIni & SPIF_SENDWININICHANGE)
		    bSendWinIniChange = TRUE;
	    }
	    break;

	case SPI_ICONHORIZONTALSPACING:
	    if (lpvParam) {
		*((LPINT)lpvParam) = GETSYSTEMMETRICS(SM_CXICONSPACING);
//    	    	APISTR((LF_APIRET,"SystemParametersInfo: returns BOOL TRUE\n"));
		return TRUE;
	    }
	    SysMetricsDef[SM_CXICONSPACING] = (int)uParam;
	    if (fuWinIni & SPIF_UPDATEINIFILE) {
		lstrcpy(lpszSection,"desktop");
		lstrcpy(szBuffer, itoa(GETSYSTEMMETRICS(SM_CXICONSPACING)));//sprintf(szBuffer,"%d",SysMetricsDef[SM_CXICONSPACING]);
		WriteProfileString(lpszSection,"IconSpacing",szBuffer);
		if (fuWinIni & SPIF_SENDWININICHANGE)
		    bSendWinIniChange = TRUE;
	    }
	    break;

	case SPI_ICONVERTICALSPACING:
	    if (lpvParam) {
		*((LPINT)lpvParam) = GETSYSTEMMETRICS(SM_CYICONSPACING);
//    	    	APISTR((LF_APIRET,"SystemParametersInfo: returns BOOL TRUE\n"));
		return TRUE;
	    }

	    SysMetricsDef[SM_CYICONSPACING] = (int)uParam;
	    if (fuWinIni & SPIF_UPDATEINIFILE) {
		lstrcpy(lpszSection,"windows");
		lstrcpy(szBuffer, itoa(GETSYSTEMMETRICS(SM_CYICONSPACING)));//sprintf(szBuffer,"%d",SysMetricsDef[SM_CYICONSPACING]);
		WriteProfileString(lpszSection,"IconSpacing",szBuffer);
		if (fuWinIni & SPIF_SENDWININICHANGE)
		    bSendWinIniChange = TRUE;
	    }
	    break;

	case SPI_LANGDRIVER:
//    	    APISTR((LF_APIRET,"SystemParametersInfo: returns BOOL FALSE\n"));
	    return FALSE;

	case SPI_SETDESKPATTERN:
//    	    APISTR((LF_APIRET,"SystemParametersInfo: returns BOOL FALSE\n"));
	    return FALSE;

	case SPI_SETDESKWALLPAPER:
//    	    APISTR((LF_APIRET,"SystemParametersInfo: returns BOOL FALSE\n"));
	    return FALSE;

	case SPI_SETDOUBLECLICKTIME:
	    nDoubleClickTime = (int)uParam;
	    if (fuWinIni & SPIF_UPDATEINIFILE) {
		lstrcpy(lpszSection,"windows");
		lstrcpy(szBuffer, itoa(nDoubleClickTime));//sprintf(szBuffer,"%d",nDoubleClickTime);
		WriteProfileString(lpszSection,"DoubleClickSpeed",szBuffer);
		if (fuWinIni & SPIF_SENDWININICHANGE)
		    bSendWinIniChange = TRUE;
	    }
	    break;

	case SPI_SETDOUBLECLKWIDTH:
	    SysMetricsDef[SM_CXDOUBLECLK] = (int)uParam;
//    	    APISTR((LF_APIRET,"SystemParametersInfo: returns BOOL TRUE\n"));
	    return TRUE;

	case SPI_SETDOUBLECLKHEIGHT:
	    SysMetricsDef[SM_CYDOUBLECLK] = (int)uParam;
//    	    APISTR((LF_APIRET,"SystemParametersInfo: returns BOOL TRUE\n"));
	    return TRUE;

	case SPI_SETMOUSEBUTTONSWAP:
	    SysMetricsDef[SM_SWAPBUTTON] = (int)uParam;
//    	    APISTR((LF_APIRET,"SystemParametersInfo: returns BOOL TRUE\n"));
	    return TRUE;

	case SPI_GETWORKAREA:
	    lpRect = (LPRECT) lpvParam;
	    if (!lpRect)
		return FALSE;
	    lpRect->left = 0;
	    lpRect->top = 0;
	    lpRect->right = GETSYSTEMMETRICS(SM_CXSCREEN);
	    lpRect->bottom = GETSYSTEMMETRICS(SM_CYSCREEN);
	    break;

	case SPI_SETWORKAREA:
//    	    APISTR((LF_APIRET,"SystemParametersInfo: returns BOOL FALSE\n"));
	    return FALSE;

	default:
//    	    APISTR((LF_APIRET,"SystemParametersInfo: returns BOOL FALSE\n"));
	    return FALSE;
    }

    if (bSendWinIniChange)
	SendMessage(HWND_BROADCAST,WM_WININICHANGE,(WPARAM)0,
		(LPARAM)lpszSection);

//    APISTR((LF_APIRET,"SystemParametersInfo: returns BOOL TRUE\n"));
    return TRUE;
}
