/*
 *  Clock (winclock.c)
 *
 *  Copyright 1998 by Marcel Baur <mbaur@g26.ethz.ch>
 *
 *  This file is based on  rolex.c  by Jim Peterson.
 *
 *  I just managed to move the relevant parts into the Clock application
 *  and made it look like the original Windows one. You can find the original
 *  rolex.c in the wine /libtest directory.
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

// C lib headers
#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <dos.h>

// Windows headers
#include "windows.h"

// Application headers
#include "winclock.h"
#include "main.h"

#define M_PI 3.1415926535 

static const int SHADOW_DEPTH = 2;
 
typedef struct
{
    POINT Start;
    POINT End;
} HandData;

static HandData HourHand, MinuteHand, SecondHand;

static void DrawTicks(HDC dc, const POINT* centre, int radius)
{
    int t;
    HPEN oldhPen, hPen;

    /* Minute divisions */
    if (radius>64)
    {
        hPen=CreatePen(PS_SOLID, 2, TickColor);
        oldhPen=SelectObject(dc, hPen);
        for(t=0; t<60; t++) {
            MoveToEx(dc,
                     centre->x + sin(t*M_PI/30)*0.9*radius,
                     centre->y - cos(t*M_PI/30)*0.9*radius,
                     NULL);
	    LineTo(dc,
		   centre->x + sin(t*M_PI/30)*0.89*radius,
		   centre->y - cos(t*M_PI/30)*0.89*radius);
	}
        SelectObject(dc, oldhPen);
        DeleteObject(hPen);
    }
    /* Hour divisions */
    hPen=CreatePen(PS_SOLID, 5, HandColor);
    oldhPen=SelectObject(dc, hPen);
    for(t=0; t<12; t++) {

        MoveToEx(dc,
                 centre->x + sin(t*M_PI/6)*0.9*radius,
                 centre->y - cos(t*M_PI/6)*0.9*radius,
                 NULL);
        LineTo(dc,
               centre->x + sin(t*M_PI/6)*0.89*radius,
               centre->y - cos(t*M_PI/6)*0.89*radius);
    }
    SelectObject(dc, oldhPen);
    DeleteObject(hPen);
}

static void DrawFace(HDC dc, const POINT* centre, int radius, int border)
{
    /* Ticks */
#if 0
// Windows 3.x doesn't  draw shadow for ticks
    SelectObject(dc, CreatePen(PS_SOLID, 2, ShadowColor));
    OffsetWindowOrgEx(dc, -SHADOW_DEPTH, -SHADOW_DEPTH, NULL);
    DrawTicks(dc, centre, radius);
    DeleteObject(SelectObject(dc, CreatePen(PS_SOLID, 2, TickColor)));
    OffsetWindowOrgEx(dc, SHADOW_DEPTH, SHADOW_DEPTH, NULL);
#endif
    DrawTicks(dc, centre, radius);
#if 0
// Windows 3.x doesn't  draw circle arout a clock
    if (border)
    {
        SelectObject(dc, GetStockObject(NULL_BRUSH));
        DeleteObject(SelectObject(dc, CreatePen(PS_SOLID, 5, ShadowColor)));
        Ellipse(dc, centre->x - radius, centre->y - radius, centre->x + radius, centre->y + radius);
    }
#endif
}

static void DrawHand(HDC dc,HandData* hand)
{
    MoveToEx(dc, hand->Start.x, hand->Start.y, NULL);
    LineTo(dc, hand->End.x, hand->End.y);
}

static void DrawHands(HDC dc, BOOL bSeconds)
{
    if (bSeconds) {
#if 0
      	SelectObject(dc, CreatePen(PS_SOLID, 1, ShadowColor));
	OffsetWindowOrgEx(dc, -SHADOW_DEPTH, -SHADOW_DEPTH, NULL);
        DrawHand(dc, &SecondHand);
	DeleteObject(SelectObject(dc, CreatePen(PS_SOLID, 1, HandColor)));
	OffsetWindowOrgEx(dc, SHADOW_DEPTH, SHADOW_DEPTH, NULL);
#else
	SelectObject(dc, CreatePen(PS_SOLID, 1, HandColor));
#endif
        DrawHand(dc, &SecondHand);
	DeleteObject(SelectObject(dc, GetStockObject(NULL_PEN)));
    }

#if 0
// Original Windows 3.x clock doesn't draw shadow for hands
    SelectObject(dc, CreatePen(PS_SOLID, 4, ShadowColor));

    OffsetWindowOrg(dc, -SHADOW_DEPTH, -SHADOW_DEPTH);
    DrawHand(dc, &MinuteHand);
    DrawHand(dc, &HourHand);
    OffsetWindowOrg(dc, SHADOW_DEPTH, SHADOW_DEPTH);

#endif
    SelectObject(dc, CreatePen(PS_SOLID, 4, HandColor));
    DrawHand(dc, &MinuteHand);
    DrawHand(dc, &HourHand);
    DeleteObject(SelectObject(dc, GetStockObject(NULL_PEN)));
}

static void PositionHand(const POINT* centre, double length, double angle, HandData* hand)
{
    hand->Start = *centre;
    hand->End.x = centre->x + sin(angle)*length;
    hand->End.y = centre->y - cos(angle)*length;
}

static void PositionHands(const POINT* centre, int radius, BOOL bSeconds)
{
//    SYSTEMTIME st;
    double hour, minute, second;
    struct dostime_t t;

    _dos_gettime (&t);
    hour = t.hour%12;
	minute = t.minute;
	second = t.second;

    /* 0 <= hour,minute,second < 2pi */


    PositionHand(centre, radius * 0.5,  ((hour*5+minute/12)/(12*5)) * 2*M_PI, &HourHand);
    PositionHand(centre, radius * 0.65, minute/60 * 2*M_PI, &MinuteHand);
    if (bSeconds)
        PositionHand(centre, radius * 0.79, second/60 * 2*M_PI, &SecondHand);  
}

void AnalogClock(HDC dc, int x, int y, BOOL bSeconds, BOOL border)
{
    POINT centre;
    int radius;
    
    radius = min(x, y)/2 - SHADOW_DEPTH;
    if (radius < 0)
	return;

    centre.x = x/2;
    centre.y = y/2;

    DrawFace(dc, &centre, radius, border);

    PositionHands(&centre, radius, bSeconds);
    DrawHands(dc, bSeconds);
}

static void IconDrawHands(HDC dc, BOOL bSeconds)
{
    SelectObject(dc, CreatePen(PS_SOLID, 1, HandColor));
    DrawHand(dc, &MinuteHand);
    DrawHand(dc, &HourHand);
    DeleteObject(SelectObject(dc, GetStockObject(NULL_PEN)));
}

static void IconDrawFace(HDC dc, const POINT* centre, int radius, int border)
{
    int t;
//    HPEN oldhPen, hPen;

    /* Hour divisions */
//    hPen=CreatePen(PS_SOLID, 2, HandColor);
//    oldhPen=SelectObject(dc, hPen);

    for(t=0; t<12; t++) {
		SetPixel(dc, centre->x + sin(t*M_PI/6)*0.88*radius, centre->y - cos(t*M_PI/6)*0.88*radius, RGB(255,255,255));
		SetPixel(dc, centre->x + sin(t*M_PI/6)*0.88*radius+1, centre->y - cos(t*M_PI/6)*0.88*radius, RGB(0,0,0));
		SetPixel(dc, centre->x + sin(t*M_PI/6)*0.88*radius+1, centre->y - cos(t*M_PI/6)*0.88*radius+1, RGB(0,0,0));
		SetPixel(dc, centre->x + sin(t*M_PI/6)*0.88*radius, centre->y - cos(t*M_PI/6)*0.88*radius+1, RGB(0,0,0));
//        MoveToEx(dc,
//                 centre->x + sin(t*M_PI/6)*0.87*radius,
//                 centre->y - cos(t*M_PI/6)*0.87*radius,
//                 NULL);
//        LineTo(dc,
//               centre->x + sin(t*M_PI/6)*0.865*radius,
//               centre->y - cos(t*M_PI/6)*0.865*radius);
    }
//    SelectObject(dc, oldhPen);
//    DeleteObject(hPen);
}


void IconAnalogClock(HDC dc, int x, int y, BOOL bSeconds, BOOL border)
{
    POINT centre;
    int radius;
    
    radius = min(x, y)/2 - SHADOW_DEPTH;
    if (radius < 0)
	return;

    centre.x = x/2;
    centre.y = y/2;

    IconDrawFace(dc, &centre, radius, border);

    PositionHands(&centre, radius, bSeconds);
    IconDrawHands(dc, bSeconds);
}

void FormatDate(char * szDate)
{
    struct dosdate_t d;
    char f[3][5]={"","",""};
	int i;
	char dFormat[20]="";
	char buf[20]="";
	
	strcpy(dFormat,"%[dMy]");
	strcat(dFormat, Globals.sDate);
	strcat(dFormat, "%[dMy]");
	strcat(dFormat, Globals.sDate);
	strcat(dFormat, "%[dMy]");

    _dos_getdate (&d);

	if (3==sscanf(Globals.sShortDate, dFormat, &f[0],&f[1],&f[2]))
	{
		for (i=0; i<3; i++)
		{
			if (!strcmp("d", f[i])) {
				sprintf(buf, "%d", d.day);
				strcat(szDate, buf);
			}
			else if (!strcmp("dd", f[i])) {
				sprintf(buf, "%02d", d.day);
				strcat(szDate, buf);
			}
			else if (!strcmp("M", f[i])) {
				sprintf(buf, "%d", d.month);
				strcat(szDate, buf);
			}
			else if (!strcmp("MM", f[i])) {
				sprintf(buf, "%02d", d.month);
				strcat(szDate, buf);
			}
			else if (!strcmp("yy", f[i])) {
				sprintf(buf, "%02d", d.year % 100);
				strcat(szDate, buf);
			}
			else if (!strcmp("yyyy", f[i])) {
				sprintf(buf, "%04d", d.year);
				strcat(szDate, buf);
			} else {
				strcat(szDate, "error");
			};
			if (i<2) strcat(szDate, Globals.sDate);
		}
	}
}
 
void FormatTime(char * szTime)
{
	char tFormat[20]="";
    struct dostime_t t;

    _dos_gettime (&t);

	if (Globals.iTLZero) 
	{
		strcpy(tFormat, "%02d");
	} else {
		strcpy(tFormat, "%d");
	}
	strcat(tFormat, Globals.sTime);
	strcat(tFormat, "%02d");
	if (Globals.bSeconds)
	{
		strcat(tFormat, Globals.sTime);
		strcat(tFormat, "%02d");
	}
	if (!Globals.iTime) // 12h
	{
		strcat(tFormat, " %s");
		if (Globals.bSeconds) 
		{
			sprintf(szTime, tFormat, (t.hour % 12)?(t.hour % 12):12, t.minute, t.second, (t.hour<12)?Globals.s1159:Globals.s2359);
		} else {
			sprintf(szTime, tFormat, (t.hour % 12)?(t.hour % 12):12, t.minute, (t.hour<12)?Globals.s1159:Globals.s2359);
		}
	} else { // 24h
		if (Globals.bSeconds) 
		{
			sprintf(szTime, tFormat, t.hour, t.minute, t.second);
		} else {
			sprintf(szTime, tFormat, t.hour, t.minute);
		}
	}

}


HFONT SizeFont(HDC dc, int x, int y, BOOL bSeconds, const LOGFONT* font)
{
    SIZE extent;
    LOGFONT lf;
    double xscale, yscale;
    HFONT oldFont, newFont;
    char szTime[255];
    int chars;

    FormatTime(szTime);
    chars=lstrlen(szTime);

    lf = *font;
    lf.lfHeight = -20;

    x -= 2 * SHADOW_DEPTH;
    y -= 2 * SHADOW_DEPTH;

    oldFont = SelectObject(dc, CreateFontIndirect(&lf));
    GetTextExtentPoint(dc, szTime, chars, &extent);
    DeleteObject(SelectObject(dc, oldFont));

    xscale = (double)x/extent.cx;
    yscale = (double)y/extent.cy;
    lf.lfHeight *= min(xscale, yscale);    
    newFont = CreateFontIndirect(&lf);

    return newFont;
}


void DigitalClock(HDC dc, int x, int y, BOOL bSeconds, HFONT font)
{
    SIZE extent;
    HFONT oldFont;
    char szTime[255]="";
    char szDate[255]="";
    int tchars;
    int dchars;

	FormatDate(szDate);
    dchars=lstrlen(szDate);


	FormatTime(szTime);
    tchars=lstrlen(szTime);


    oldFont = SelectObject(dc, font);

    GetTextExtentPoint(dc, szTime, tchars, &extent);

    SetBkColor(dc, BackgroundColor);
    SetTextColor(dc, ShadowColor);
    TextOut(dc, (x - extent.cx)/2 + SHADOW_DEPTH, (y - extent.cy)/2 + SHADOW_DEPTH, szTime, tchars);
    SetBkMode(dc, TRANSPARENT);

    SetTextColor(dc, HandColor);
    TextOut(dc, (x - extent.cx)/2, (y - extent.cy)/2, szTime, tchars);

//    GetTextExtentPoint(dc, szDate, dchars, &extent);
//    TextOut(dc, (x - extent.cx)/2, (y - extent.cy)/2, szDate, dchars);

    SelectObject(dc, oldFont);
}
