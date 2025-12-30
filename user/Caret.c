/*    
	Caret.c	2.12
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

 */

#include "user.h"

//#include "kerndef.h"

// This  is undoc message, but known by many...
#define WM_SYSTIMER 0x0118

// Some tricks to use export names... This is staring from 3.1. Under 3.0 name is normal.

#define KillSystemTimer BEAR182
BOOL WINAPI KillSystemTimer(HWND hWindow, WORD wTimer);

#define SetSystemTimer BEAR11
WORD WINAPI SetSystemTimer(HWND hWnd, int nIDEvent, WORD wElapse, FARPROC lpTimerProc);

typedef struct {
	HWND	hWnd;
	HANDLE  hBitmap;
	BOOL	visible;	/* if currently on  */
	UINT	blinkrate;
	int	onoff;		/* #show vs. #hides */
	int	Height,Width;   /* of hBitmap (if any) */
	int	x,y;		/* where it is */
	int	timer;
} CARET;

CARET caret; /* we have the only caret in the system */

/* caret messages */
#define	CARET_CREATE		0
#define CARET_DESTROY		1
#define CARET_POSITION		2
#define CARET_SHOWHIDE		3

/* caret state */
#define OFF	0
#define ON	1

/* internal definitions */
static WORD CALLBACK InternalCaretFunc(HWND , WORD ,int , DWORD );
static void CaretDrv(int, LONG, DWORD);
static void DrawCaret(HDC,int);

/************************************************************************/
/* DrawCaret()								*/
/*	calculate the state given, what it is and what is desired	*/
/*	turn it on/off or leave it alone on/off				*/
/*									*/
/************************************************************************/
static void
DrawCaret(HDC hDC,int turnon)
{
	HDC	hCaretDC = hDC;
	HDC	hBitDC;
	HRGN	hrgnClip ;
	RECT	rcClient ;

	FUNCTION_START

	if (caret.hWnd == 0)
           return;

	GetClientRect(caret.hWnd, &rcClient) ;

	if (caret.visible != turnon) 
           {
	   /*   Transition from on->off or off->on 
           */
	   if (hDC == 0)
              hCaretDC = GetDC(caret.hWnd);
	   hrgnClip = CreateRectRgnIndirect(&rcClient) ;
	   SelectClipRgn(hCaretDC, hrgnClip) ;
	   if (caret.hBitmap)
	      {
              hBitDC = CreateCompatibleDC(hCaretDC) ;
              SelectObject(hBitDC, caret.hBitmap) ;
              BitBlt(hCaretDC, caret.x, caret.y, caret.Width, caret.Height, hBitDC, 0, 0, SRCINVERT) ;
              DeleteDC(hBitDC) ;
              }
           else PatBlt(hCaretDC, caret.x, caret.y, caret.Width, caret.Height, DSTINVERT) ;
           DeleteObject(hrgnClip);
           if (hDC == 0)
              ReleaseDC(caret.hWnd,hCaretDC);
           }
        caret.visible = turnon ;
	/*   
        **   Turn on or off timers as needed 
        */
	if (caret.onoff > 0) 
           {
/*--- #if 0 ---*/
           /*   If no timer, set one up 
           */
           if (caret.timer == 0)
              caret.timer = SetSystemTimer(
				caret.hWnd,1,
				caret.blinkrate,
				(TIMERPROC)InternalCaretFunc);
/*--- #endif ---*/
           } 
        else {
/*--- #if 0 ---*/
             /* 
             **   If timer active, kill it 
             */
             if (caret.timer)
                KillSystemTimer(caret.hWnd, caret.timer);
/*--- #endif ---*/
             caret.timer = 0;
             }
}

/* this is the callback from the system clock       */
/* it exists solely to call CaretDrv to toggle the caret */

static WORD CALLBACK
InternalCaretFunc(HWND hWnd, WORD wMsg,int nId, DWORD dwTime)
{
	FUNCTION_START

	if(wMsg == WM_SYSTIMER)
		/* this will toggle the caret */
		CaretDrv(CARET_SHOWHIDE,0,-1);
	return 0;
}

/* this code could be moved into the driver 	*/
/* this is the 'driver' for the caret 		*/
/* this will create/destroy show and move it    */

static void
CaretDrv(int msg, LONG flag,DWORD lParam)
{
	CARET  *lpcaret;
	LPPOINT lppt;

	FUNCTION_START

	switch(msg) {
		case CARET_CREATE:
			lpcaret = (CARET *) lParam;		
			caret = *lpcaret;
			caret.visible = 0;
			caret.onoff   = 0;
			caret.x	   = 0;
			caret.y	   = 0;
			caret.timer   = 0;
			caret.blinkrate = 500;
			break;

		case CARET_DESTROY:
			caret.onoff      = 0;
			DrawCaret((HDC)lParam,OFF);
			caret.hWnd = 0;
			break;

		case CARET_POSITION:
			lppt = (LPPOINT) lParam;
			if(flag) {
				lppt->x = caret.x;
				lppt->y = caret.y;
			} else {
				/* if on hide it */
				if (caret.visible)
					DrawCaret(0, OFF);

				caret.x = lppt->x;
				caret.y = lppt->y;
				if(caret.onoff > 0)
					DrawCaret(0, ON) ;
			}
			break;
		case CARET_SHOWHIDE:
			if (lParam == -1)
				DrawCaret(0, !caret.visible) ;
			else if (lParam == (DWORD)caret.hWnd)
			{
				caret.onoff += flag ;
				DrawCaret(0, caret.onoff > 0) ;
			}
			break ;
	}	
}

/************************************************************************/
/*									*/
/*	Caret API's							*/
/*		These are all exported.					*/
/*									*/
/*		package up the request and call the driver		*/
/*									*/
/************************************************************************/

/**************************************************************************
 *              CreateCaret   (USER.163)
 */
void WINAPI
CreateCaret(HWND hWnd, HBITMAP hBitmap, int nWidth, int nHeight)
{
	BITMAP	bitmap ;
	CARET	new;
	new.hWnd 	= hWnd;
	new.hBitmap 	= hBitmap;	
	new.Width	= nWidth;
	new.Height	= nHeight;

	FUNCTION_START
	
	if (new.hBitmap)
	{
		GetObject(new.hBitmap, sizeof(bitmap), &bitmap) ;
		new.Width = bitmap.bmWidth ;
		new.Height = bitmap.bmHeight ;
	}

	if (!new.Width)
		new.Width = GetSystemMetrics(SM_CXBORDER) ;

	if (!new.Height)
		new.Height = GetSystemMetrics(SM_CYBORDER) ;
	
	CaretDrv(CARET_CREATE, 0, (DWORD) &new);
}


/*****************************************************************
 *		DestroyCaret (USER.164)
 */
void WINAPI
DestroyCaret(void)
{
	FUNCTION_START
	CaretDrv(CARET_DESTROY,0,0);
}

/*****************************************************************
 *		SetCaretPos (USER.165)
 */
void WINAPI
SetCaretPos(int X, int Y)
{
	POINT	caretpos;

	FUNCTION_START

	caretpos.x = X;
	caretpos.y = Y;
	CaretDrv(CARET_POSITION,0,(DWORD) &caretpos);
}

/**************************************************************************
 *              HideCaret   (USER.166)
 */
void WINAPI
HideCaret(HWND hWnd)
{
	FUNCTION_START
	CaretDrv(CARET_SHOWHIDE,-1,(DWORD)hWnd);
}

/**************************************************************************
 *              ShowCaret   (USER.167)
 */
void WINAPI
ShowCaret(HWND hWnd)
{
	FUNCTION_START
	CaretDrv(CARET_SHOWHIDE, 1,(DWORD)hWnd);
}

/*****************************************************************
 *		SetCaretBlinkTime (USER.168)
 */
void WINAPI
SetCaretBlinkTime(UINT	blinkrate)
{
	FUNCTION_START
	caret.blinkrate = blinkrate;

	/* turn on or off timers as needed */
	if(caret.timer == 0)
		return;

/*--- #if 0 ---*/
// KillSystemTimer exported as BEAR182 by USER.182 !!BEWARE!! Same function name used by SYSTEM.DRV!!!
	KillSystemTimer(caret.hWnd, caret.timer);
// SetSystemTimer exported as BEAR11 by USER.11
	caret.timer = SetSystemTimer(
				caret.hWnd,1,caret.blinkrate,
				(TIMERPROC)InternalCaretFunc);
/*--- #endif ---*/
	FUNCTION_END
}

/*****************************************************************
 *		GetCaretBlinkTime (USER.169)
 */
UINT WINAPI
GetCaretBlinkTime()
{
	FUNCTION_START
	if ( caret.blinkrate == 0 )
		caret.blinkrate = 500;

	return caret.blinkrate;
}

/*****************************************************************
 *		GetCaretPos (USER.183)
 */
void WINAPI
GetCaretPos(LPPOINT lpPoint)
{
	FUNCTION_START
	CaretDrv(CARET_POSITION,1,(DWORD) lpPoint);

}
