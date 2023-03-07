#include <windows.h>

int   WINAPI
DrawText(HDC hDC, LPCSTR lpsz, int cb, LPRECT lprc, UINT uFormat)
{
	TEXTMETRIC TextMetrics;
	DWORD 	extent;

	int     TextWidth;
	int     cnt,dx,dy;
	int     space;
	int     place = 0;
	int 	nlines;

	int   	Ampersands;
	int   	AmpersandPos;

	int   	TabStops = 8;
	int	LineHeight;
	int	rtotal = 0;
	int	lbreak;
	int 	x,y,baseline;
	HPEN    hPen = 0;

	char far *lpstr = (char far *)lpsz;

	int   	charwidth[256];

	if ( !lprc )
		return 0;

//	LOGSTR((LF_API,
//	    "DrawText: (hdc:%x str:[%s],len:%d rect:%d,%d:%d,%d flags=%x)\n",
//		hDC,GdiDumpString((LPSTR)lpsz,cb),cb,
//		lprc->left,lprc->top,lprc->right,lprc->bottom,
//		uFormat));

	/****************************************/
	/* initialize the text engine 		*/
	/****************************************/

	/* number chars per tabstop */
	if(uFormat & DT_TABSTOP) {
		TabStops = (uFormat >> 8);
		uFormat &= 0xff;
	}

	/* pointer to string... */
	lpstr  = (LPSTR)lpsz;

	/* number of characters to output */
	if(cb == -1)
		cb = lstrlen(lpstr);

	/* get character width array */
	GetCharWidth(hDC, 0,255, charwidth);

	GetTextMetrics(hDC,(LPTEXTMETRIC) &TextMetrics);
	LineHeight = TextMetrics.tmHeight;

	/* additional line spacing */
	if(uFormat & DT_EXTERNALLEADING) {
		LineHeight += TextMetrics.tmExternalLeading;
	}

	if (uFormat & DT_CALCRECT) {
	    lprc->left = HIWORD(lprc->left)?0:lprc->left;
	    lprc->right = HIWORD(lprc->right)?0:lprc->right;
	    lprc->top = HIWORD(lprc->top)?0:lprc->top;
	    lprc->bottom = HIWORD(lprc->bottom)?0:lprc->bottom;
	}
	else {	/* we need to actually draw */
	    /* Create solid pen of text color for drawing underlines */
	    hPen = CreatePen(PS_SOLID,1,GetTextColor(hDC));
	    hPen = SelectObject(hDC,hPen);
	}

	/* get the width of the line to draw into... */
	TextWidth = lprc->right - lprc->left;
	
	baseline= lprc->top;

	nlines = 0;

	/****************************************/
	/* loop over all lines 			*/
	/****************************************/
	while(cb) {

		/****************************************/
		/* get a line, w/ tabs and prefix 	*/
		/****************************************/
		space = 0;
		Ampersands = 0;		/* space used by _ to remove */
		AmpersandPos = 0;	/* position of last Ampersand */
		rtotal	   = 0;		/* running width of the line */
		lbreak	   = 0;		/* have we a line break      */

		/* get characters till line break */
		for(cnt=0;cnt < cb;cnt++) {	
			
			switch(lpstr[cnt]) {
			case 0:
				lbreak++;
				break;
			case ' ':
				space++;
				place = cnt;
				rtotal += charwidth[(BYTE)lpstr[cnt]];
				break;
			case '\t':
				space++;
				place = cnt;
				break;
			case '&':
				if(uFormat & DT_NOPREFIX)
					rtotal += charwidth[(BYTE)lpstr[cnt]];
				else {
				    if ( Ampersands && AmpersandPos+1 == cnt) {
					rtotal += charwidth[(BYTE)lpstr[cnt]];
				    }
				    else {
					AmpersandPos=cnt;
					Ampersands=1;
				    }
				}
				break;
			case '\r':
			case '\n':
				lbreak++;
				break;	
			default:
				rtotal += charwidth[(BYTE)lpstr[cnt]];
				break;
			}	
		
			/* did we hit eol?			*/
			if(lbreak)
				break;
		
			/* if too wide and calcrect, adjust width... */
			if(rtotal > TextWidth) {

				if(uFormat & DT_CALCRECT &&
				   uFormat & DT_SINGLELINE) {
					lprc->right += (rtotal - TextWidth);
					TextWidth = lprc->right - lprc->left;
					continue;
				}
				if(uFormat & DT_WORDBREAK) {
					/* did we have a wordbreak yet? */
				  if(space) {
				    cnt = place;
				    /* Recalc text len from last word */
				    extent = GetTextExtent(hDC, lpstr, cnt-1);
				    rtotal = LOWORD(extent);
				  }
				  cnt++;
				  break;
				}
			}
	 	}

		/**************************************/
		/* set width of the string 	      */
		/**************************************/

		dx =  rtotal;
		dy =  LineHeight;

		if(uFormat & DT_RIGHT)
			x = lprc->right - dx + 1;
		else if(uFormat & DT_CENTER)
			x = (int)(lprc->left + lprc->right - dx)/2;
		else    x = lprc->left;

		/****************************************/
		/* set height of the string 		*/
		/****************************************/
		if(uFormat & DT_VCENTER) {
			extent = GetTextExtent(hDC, lpstr, cnt);
			dy = HIWORD(extent);
		    	y = (int)(baseline + lprc->bottom - dy)/2;
		} else if(uFormat & DT_BOTTOM)
		    	y = lprc->bottom - dy;
		else    y = baseline;

		/****************************************/
		/* do we actually draw 			*/
		/****************************************/
		if((uFormat & DT_CALCRECT) == 0) {
			/* more complex w/ prefix handling */
			if(Ampersands)
				DrawTextOut(hDC,x,y,lpstr,cnt,uFormat);
			else {
				if(uFormat & DT_EXPANDTABS) {
					TabbedTextOut(hDC,x,y,lpstr,cnt,0,0,x);
				} else {
				    if (uFormat & DT_NOCLIP)
					TextOut(hDC,x,y,lpstr,cnt);
				    else
					ExtTextOut(hDC,x,y,
						ETO_CLIPPED,lprc,
						lpstr,cnt,NULL);
				}
			}
		}

		/****************************************/
		/* adjust for the next line		*/
		/****************************************/
		nlines++;
		baseline += LineHeight;

		/* adjust for calculations... */
		if(uFormat & DT_CALCRECT) {
			lprc->bottom = baseline;
		}

		if(uFormat & DT_SINGLELINE)
			break;

		/* skip over any newline... */
		if(lbreak) {
			if(lpstr[cnt] == '\r')
				cnt++;
			if(lpstr[cnt] == '\n')
				cnt++;
		}

		/* setup for next line */
		lpstr += cnt;
		cb -= cnt;

		/* if we would overflow the rectangle then quit */
		/* allow for another complete line, ie. space	*/
		if(baseline > lprc->bottom) {
			break;
		}

	}

	if ((uFormat & DT_CALCRECT) &&
	    !(uFormat & DT_SINGLELINE) &&
	    (nlines == 1))
	    lprc->right = rtotal;

	if (hPen) {
	    hPen = SelectObject(hDC,hPen);
	    DeleteObject(hPen);
	}

	return baseline - lprc->top;	
}
