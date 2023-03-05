/*    
	SysMetrics.c	2.32
    	Copyright 1997 Willows Software, Inc. 

This library is free software; you can redistribute it and/or
modify it under the terms of the GNU Library General Public License as
published by the Free Software Foundation; either version 2 of the
License, or (at your option) any later version.

This library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Library General Public License for more details.

You should have received a copy of the GNU Library General Public
License along with this library; see the file COPYING.LIB.  If
not, write to the Free Software Foundation, Inc., 675 Mass Ave,
Cambridge, MA 02139, USA.


For more information about the Willows Twin Libraries.

	http://www.willows.com	

To send email to the maintainer of the Willows Twin Libraries.

	mailto:twin@willows.com 

   This file contains for the sysmetrics-related API functions.    
 */

#include <stdio.h>
#include <string.h>

#include <windows.h>

//#include "SysMetrics.h"

//#include "kerndef.h"
//#include "Kernel.h"
//#include "Log.h"
//#include "WinConfig.h"

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


static int SysMetricsDef[] =
  {
        640,            /* SM_CXSCREEN         0 */
        480,            /* SM_CYSCREEN         1 */
        20,             /* SM_CXVSCROLL        2 */
        20,             /* SM_CYHSCROLL        3 */
        25,             /* SM_CYCAPTION        4 */
        1,              /* SM_CXBORDER         5 */
        1,              /* SM_CYBORDER         6 */
        1 /*4*/,        /* SM_CXDLGFRAME       7 */
        1 /*4*/,        /* SM_CYDLGFRAME       8 */
        20,             /* SM_CYVTHUMB         9 */
        20,             /* SM_CXHTHUMB         10 */
        32,             /* SM_CXICON           11 */
        32,             /* SM_CYICON           12 */
        32,             /* SM_CXCURSOR         13 */
        32,             /* SM_CYCURSOR         14 */
        25,             /* SM_CYMENU           15 */
	/* NOTE: -1 forces GetSystemMetrics() to compute value once 
	 * and replace -1 with computed value.
	 */
        640,            /* SM_CXFULLSCREEN     16 */
        460,            /* SM_CYFULLSCREEN     17 */
        0,              /* SM_CYKANJIWINDOW    18 */
        1,              /* SM_MOUSEPRESENT     19 */
        20,             /* SM_CYVSCROLL        20 */
        20,             /* SM_CXHSCROLL        21 */
        0,              /* SM_DEBUG            22 */
        0,              /* SM_SWAPBUTTON       23 */
        0,              /* SM_RESERVED1        24 */
        1,              /* SM_RESERVED2        25 */
        0,              /* SM_RESERVED3        26 */
        0,              /* SM_RESERVED4        27 */
        102,            /* SM_CXMIN            28 */
        26,             /* SM_CYMIN            29 */
        18,             /* SM_CXSIZE           30 */
        18,             /* SM_CYSIZE           31 */
        4,              /* SM_CXFRAME          32 */
        4,              /* SM_CYFRAME          33 */
        102,            /* SM_CXMINTRACK       34 */
        26,             /* SM_CYMINTRACK       35 */
	5,		/* SM_CXDOUBLECLK      36 */
	4,		/* SM_CYDOUBLECLK      37 */
	48,		/* SM_CXICONSPACING    38 */
	32,		/* SM_CYICONSPACING    39 */
	0,		/* SM_MENUDROPALIGNMENT 40 */
	0,		/* SM_PENWINDOWS       41 */
	0,		/* SM_DBCSENABLED      42 */
	3,		/* SM_CMOUSEBUTTONS    43 */
	/* Win95 */
	FALSE,		/* SM_SECURE		44 */
	2,		/* SM_CXEDGE		45 */
	2,		/* SM_CYEDGE		46 */
	48,		/* SM_CXMINSPACING	47 */
	48,		/* SM_CYMINSPACING	48 */
	16,		/* SM_CXSMICON		49 */
	16,		/* SM_CYSMICON		50 */
	16,		/* SM_CYSMCAPTION	51 */
	16,		/* SM_CXSMSIZE		52 */
	16,		/* SM_CYSMSIZE		53 */
	25,		/* SM_CXMENUSIZE	54 */
	25,		/* SM_CYMENUSIZE	55 */
	ARW_BOTTOMLEFT|ARW_LEFT, /* SM_ARRANGE		56 */
	32,		/* SM_CXMINIMIZED	57 */
	32,		/* SM_CYMINIMIZED	58 */
	640,		/* SM_CXMAXTRACK	59 */
	480,		/* SM_CYMAXTRACK	60 */
	640,		/* SM_CXMAXIMIZED	61 */
	480,		/* SM_CYMAXIMIZED	62 */
	0,		/* SM_NETWORK		63 */
	0,		/* SM_CLEANBOOT		64 */
	4,		/* SM_CXDRAG		68 */
	4,		/* SM_CYDRAG		69 */
	/* all versions */
	FALSE,		/* SM_SHOWSOUNDS	70 */
	/* Win95 */
	16,		/* SM_CXMENUCHECK	71 */
	16,		/* SM_CYMENUCHECK	72 */
	FALSE,		/* SM_SLOWMACHINE	73 */
	FALSE,		/* SM_MIDEASTENABLED	74 */
	/* all versions */
	75,		/* SM_CMETRICS		75 */
  };

static COLORREF SysColors[] =
  {
	RGB(192,192,192),	/* COLOR_SCROLLBAR */
	RGB(255,255,255),	/* COLOR_BACKGROUND */
	RGB(0,0,192),		/* COLOR_ACTIVECAPTION */
	RGB(255,255,255),	/* COLOR_INACTIVECAPTION */
	RGB(255,255,255),	/* COLOR_MENU */
	RGB(255,255,255),	/* COLOR_WINDOW */
	RGB(0,0,0),		/* COLOR_WINDOWFRAME */
	RGB(0,0,0),		/* COLOR_MENUTEXT */
	RGB(0,0,0),		/* COLOR_WINDOWTEXT */
	RGB(255,255,255),	/* COLOR_CAPTIONTEXT */
	RGB(192,192,192),	/* COLOR_ACTIVEBORDER */
	RGB(192,192,192),	/* COLOR_INACTIVEBORDER */
	RGB(192,192,192),	/* COLOR_APPWORKSPACE */
	RGB(255,0,0),		/* COLOR_HIGHLIGHT */
	RGB(255,255,255),	/* COLOR_HIGHLIGHTTEXT */
	RGB(192,192,192),	/* COLOR_BTNFACE */
	RGB(128,128,128),	/* COLOR_BTNSHADOW */
	RGB(128,128,128),	/* COLOR_GRAYTEXT */
	RGB(0,0,0),		/* COLOR_BTNTEXT */
	RGB(0,0,0),		/* COLOR_INACTIVECAPTIONTEXT */
	RGB(255,255,255),	/* COLOR_BTNHIGHLIGHT */
	/* WIN32 */
	RGB(0,0,0),		/* (21) COLOR_3DDKSHADOW */
	RGB(192,192,192),	/* (22) COLOR_3DLIGHT */
	RGB(0,0,0),		/* (23) COLOR_INFOTEXT */
	RGB(255,255,0),		/* (24) COLOR_INFOBK */
  };

/***********************************************************************
 *		GetSystemMetrics (USER.179)
 */
int WINAPI
GetSystemMetrics(int nIndex)
{
    int	rc;

//    APISTR((LF_APICALL,"GetSystemMetrics(int=%d)\n", nIndex));

    /* get default system metrics value */
    if ((nIndex < 0)
     || (nIndex > sizeof(SysMetricsDef)/sizeof(SysMetricsDef[0])))
    {
//	APISTR((LF_APIFAIL, "GetSystemMetrics: returns 0\n"));
	return 0;
    }
    rc = SysMetricsDef[nIndex];

    /* compute system metrics value if no default */
    if (rc == -1)
    {
	switch (nIndex)
	{
#ifdef LATER
	case CYMENU:
	    {
		HDC hDC;
		TEXTMETRIC tm;

		hDC = GetDC((HWND) 0);
		if (!GetTextMetrics(hDC, &tm))
		{
			rc = 25;
		}
		else
		{
			rc = tm.tmHeight + 2 * tm.tmExternalLeading;
		}
		ReleaseDC((HWND) 0, hDC);
	    }
	    break;
#endif
	default:
//	    APISTR((LF_APIFAIL, "GetSystemMetrics: returns 0\n"));
	    return 0;
	}
    }

    /* return system metrics value */
//    APISTR((LF_APIRET, "GetSystemMetrics: returns %d\n", rc));
    return rc;

}

/*************************************************************************
 *		GetSysColor (USER.180)
 */
COLORREF WINAPI
GetSysColor(int nIndex)
{
    COLORREF cr;

//    APISTR((LF_APICALL,"GetSysColor(int=%d)\n", nIndex));

    if ((nIndex < 0) || (nIndex >= sizeof(SysColors)/sizeof(SysColors[0])))
    {
	cr = RGB(0,0,0);
//        APISTR((LF_APIFAIL,"GetSysColor: returns COLORREF %x\n", cr));
        return cr;
    }

    cr = SysColors[nIndex];

//    APISTR((LF_APIRET,"GetSysColor: returns COLORREF %x\n", cr));
    return cr;
}

/*************************************************************************
 *		SetSysColors (USER.181)
 */
void WINAPI
SetSysColors(int nChanges, const int far *lpSysColor,
	     const COLORREF FAR *lpColorValues)
{
	int index;

//	APISTR((LF_APICALL,"SetSysColors(int=%d,int *%x,COLORREF *%x)\n",
//		nChanges,lpSysColor,lpColorValues));

	while(nChanges--) {
		index = *lpSysColor++;
		SysColors[index] = *lpColorValues++;
//@ todo This part is present in later windows versions
/*		if (SysColorBrushes[index] != (HBRUSH) NULL) {
			DeleteObject(SysColorBrushes[index]);
			SysColorBrushes[index] = 0;
		}*/
	}

	/* then tell top level windows of change */
	SendMessage(HWND_BROADCAST,WM_SYSCOLORCHANGE,0,0);
//	APISTR((LF_APIRET,"SetSysColors: returns void\n"));
}
