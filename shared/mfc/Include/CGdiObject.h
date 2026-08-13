/************************************************************************
  T h e   O p e n   F o u n d a t i o n    C l a s s e s
 ------------------------------------------------------------------------
  Filename   : CGdiObject.h
  Version    : 0.20
  Author(s)  : William D. Herndon

 --[ Description ]-------------------------------------------------------
  This file is part of the OFC - Open Foundation Classes -.
  It is MFC compatible.
  This file is the interface to the GDI object classes:
  CGdiObject, CBitmap, CBrush, CFont, CPen and CDC.
  All of these objects except CDC are based on CGdiObject, allowing
  the basic functions, such as Attach(), Detach(), DeleteObject().
  Most of the function implementations are so simple that they
  are implemented inline in CGdiObject_inline.h

 --[ History ] ----------------------------------------------------------

cb  = Carsten Breuer     (Carsten.Breuer@breuer-software.de)
gv  = Geurt Vos          (geurt@users.sourceforge.net)
id  = Ivan Deras         (ideras@users.sourceforge.net)
tm  = Tim Musschoot      (timussch@users.sourceforge.net)
wdh = William D. Herndon (shadowdog@users.sourceforge.net)

mm-dd-yy  ver   who  what
11-01-03  0.10  wdh  First created.
11-09-03  0.20  wdh  Added more methods of creating/loading GDI objects,
     added FromHandle() functionality (including GetObjHandle() and Attach)

 --[ How to compile ]----------------------------------------------------

  This file was developed under DevC++ 4

 --[ License ] ----------------------------------------------------------

  This program is free software; you can redistribute it and/or
  modify it under the terms of the GNU General Public License
  as published by the Free Software Foundation; either version 2
  of the License, or (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

 ------------------------------------------------------------------------
  Copyright (c) 2000-03 Open Foundation Classes
  Copyright (c) 2003 William D. Herndon
 ************************************************************************/

#ifndef CGDIOBJECT_H
#define CGDIOBJECT_H

class CDC;

class CGdiObject : public CObject {
	DECLARE_DYNCREATE(CGdiObject)

protected:
	HGDIOBJ m_hObject;
public:
static	CGdiObject* FromHandle(HGDIOBJ hObj);

	CGdiObject();
	virtual	~CGdiObject();

	operator HGDIOBJ() const;

	// Virtual function overrides
	virtual HANDLE GetObjHandle() const;
	virtual BOOL Attach(HANDLE hObject);

	// Management Functions
	HGDIOBJ Detach();
	BOOL DeleteObject();
	int GetObject(int nCount,LPVOID pObj) const;
	BOOL CreateStockObject(int nIndex);
};

class CBitmap : public CGdiObject {
	DECLARE_DYNAMIC(CBitmap)

	CBitmap();

	BOOL CreateBitmap(int iWidth,int iHeight,UINT nPlanes,
		UINT nDepth,const void* pBits);
	BOOL LoadBitmap(LPCTSTR pszRsc);
	BOOL LoadBitmap(UINT nRscID);
	BOOL LoadOEMBitmap(UINT nIDBitmap); // for system bitmaps: OBM_*
	BOOL CreateBitmapIndirect(LPBITMAP pBmp);
	BOOL CreateCompatibleBitmap(CDC* pDC,int iWidth,int iHeight);
	operator HBITMAP() const;
	int GetBitmap(LPBITMAP pBmp);
};

class CBrush : public CGdiObject {
	DECLARE_DYNAMIC(CBrush)

	CBrush();
	CBrush(COLORREF crColor);
	BOOL CreateSolidBrush(COLORREF crColor);
	BOOL CreateBrushIndirect(LPLOGBRUSH pBrush);
	BOOL CreateHatchBrush(int iIdx,COLORREF crHatch);
	BOOL CreatePatternBrush(CBitmap* pBitmap);
	BOOL CreateDIBPatternBrush(HANDLE hDIB,UINT wColSpec);
};

class CFont : public CGdiObject {
	DECLARE_DYNAMIC(CFont)

	CFont();

	BOOL CreateFontIndirect(const LOGFONT* pLogFont);
	BOOL CreateFont(int iHeight,int iWidth,int iEscape,int iOrient,
		int iWeight,BYTE byItalic,BYTE byUnder,BYTE byStrike,
		BYTE byCharSet,BYTE byOutPrec,BYTE byClipPrec,BYTE byQuality,
		BYTE byPitchAndFamily,LPCSTR pszFaceName);
};

class CPen : public CGdiObject {
	DECLARE_DYNAMIC(CPen)

	CPen();
	CPen(int iStyle,int iWidth,COLORREF crColor);
	BOOL CreatePen(int iStyle,int iWidth,COLORREF crColor);
	BOOL CreatePenIndirect(LPLOGPEN pPen);
};


class CDC : public CObject {
	DECLARE_DYNCREATE(CDC)

	HDC m_hDC; // The device context handle (must be first data member)

static	CDC* FromHandle(HDC hdc);
	CDC();
	virtual ~CDC();

	operator HDC() const;

	// Virtual function overrides
	virtual HANDLE GetObjHandle() const;
	virtual BOOL Attach(HANDLE hObj);

	// Management Functions
	HDC Detach();
	BOOL CreateCompatibleDC(CDC* pDC);
	BOOL CreateDC(LPCSTR pszDriver,LPCSTR pszDevice,
		LPCSTR pszOutput,LPVOID pInitData);
	BOOL CreateIC(LPCSTR pszDriver,LPCSTR pszDevice,
		LPCSTR pszOutput,LPVOID pInitData);
	BOOL DeleteDC();

	// Attribute functions
	CBitmap* SelectObject(CBitmap* pBmp);
	CBrush* SelectObject(CBrush* pBrush);
	CFont* SelectObject(CFont* pFont);
	CPen* SelectObject(CPen* pPen);
	COLORREF SetTextColor(COLORREF crColor);
	COLORREF SetBkColor(COLORREF crColor);
	int SetBkMode(int iMode);
	int SetPolyFillMode(int iMode);
	int SetROP2(int iMode);
	int SetStretchBltMode(int iMode);

	// Mapping functions
	int GetMapMode() const;
	CPoint GetViewportOrg() const;
	int SetMapMode(int iMode);
	CPoint SetViewportOrg(int x,int y);
	CPoint SetViewportOrg(POINT pt);
	CSize GetViewportExt() const;
	CSize SetViewportExt(int cx,int cy);
	CSize SetViewportExt(SIZE sizExt);
	CPoint GetWindowOrg() const;
	CPoint SetWindowOrg(int x,int y);
	CPoint SetWindowOrg(POINT pt);
	CSize GetWindowExt() const;
	CSize SetWindowExt(int cx,int cy);
	CSize SetWindowExt(SIZE sizExt);

	void DPtoLP(LPPOINT pPts,int iCount = 1) const;
	void DPtoLP(LPRECT prc) const;
	void DPtoLP(LPSIZE psiz) const;
	void LPtoDP(LPPOINT pPts,int iCount = 1) const;
	void LPtoDP(LPRECT prc) const;
	void LPtoDP(LPSIZE psiz) const;

	// Drawing functions
	BOOL Arc(int x1,int y1,int x2,int y2,int x3,int y3,int x4,int y4);
	BOOL Arc(LPCRECT prc,POINT ptBeg,POINT ptEnd);
	BOOL BitBlt(int xDst,int yDst,int iWidth,int iHeight,CDC* pSrcDC,
		int xSrc,int ySrc,DWORD dwRop);
	BOOL Ellipse(int x1,int y1,int x2,int y2);
	BOOL Ellipse(LPCRECT lpRect);
	BOOL Pie(int x1,int y1,int x2,int y2,int x3,int y3,int x4,int y4);
	BOOL Pie(LPCRECT lpRect, POINT ptStart, POINT ptEnd);
	BOOL Polygon(LPPOINT lpPoints,int nPts);
	BOOL PolyPolygon(LPPOINT pPts,LPINT pCnts,int nCnts);
	BOOL Polyline(LPPOINT pPts,int nPts);
//	BOOL PolyPolyline(LPPOINT pPts,LPDWORD pCnts,int nCnts);
	BOOL Rectangle(int x1, int y1, int x2, int y2);
	BOOL Rectangle(LPCRECT lpRect);
	BOOL RoundRect(int x1, int y1, int x2, int y2, int x3, int y3);
	BOOL RoundRect(LPCRECT lpRect, POINT point);

	// Text Functions
	BOOL TextOut(int x,int y,LPCSTR pszText,int nChars);
	BOOL TextOut(int x,int y,const CString& str);
	BOOL ExtTextOut(int x,int y,UINT nOptions,LPCRECT prc,
		LPCSTR pszText,UINT nChars,LPINT pDx);
	BOOL ExtTextOut(int x,int y,UINT nOptions,LPCRECT prc,
		const CString& str,LPINT pDx);
	CSize TabbedTextOut(int x,int y,LPCSTR pszText,int nChars,
		int nTabPos,LPINT pnTabStopPos,int nTabOrigin);
	CSize TabbedTextOut(int x,int y,const CString& str,
		int nTabPos,LPINT pnTabStopPos,int nTabOrigin);
	int DrawText(LPCSTR pszText,int nChars,LPRECT prc,UINT wFlags);
	int DrawText(const CString& str,LPRECT prc,UINT wFlags);

	// Path Functions
//	BOOL AbortPath();
//	BOOL ArcTo(int x1,int y1,int x2,int y2,int x3,int y3,int x4,int y4);
//	BOOL ArcTo(LPCRECT prc,POINT ptBeg,POINT ptEnd);
//	BOOL BeginPath();
	BOOL Chord(int x1,int y1,int x2,int y2,int x3,int y3,int x4,int y4);
	BOOL Chord(LPCRECT prc,POINT ptBeg,POINT ptEnd);
//	BOOL CloseFigure();
//	BOOL EndPath();
//	BOOL FillPath();
//	BOOL FlattenPath();
	void LineTo(int x,int y);
	void LineTo(POINT pt);
	void MoveTo(int x,int y);
	void MoveTo(POINT pt);
//	BOOL PolyBezier(const POINT* pPts,int nPts);
//	BOOL PolyBezierTo(const POINT* pPts,int nPts);
//	BOOL PolyDraw(const POINT* pPts,const BYTE* pTypes,int nPts);
//	BOOL PolylineTo(LPPOINT pPts,int nPts);
//	BOOL StrokeAndFillPath();
//	BOOL StrokePath();
//	BOOL WidenPath();
//	float GetMiterLimit() const;
//	BOOL SetMiterLimit(float fLimit);
//	int GetPath(LPPOINT pPts,LPBYTE pTypes,int nPts) const;

	// Info Functions
	int GetClipBox(LPRECT prc);
	int GetDeviceCaps(int nIndex) const;
	int EnumObjects(int iObjType,
		void (CALLBACK* pfn)(LPVOID,LPARAM),LPARAM pData);
	CSize GetTextExtent(LPCSTR pszText,int nChars) const;
	CSize GetTextExtent(const CString& str) const;

	// Miscellaneous functions
	void FillRect(LPCRECT prc,CBrush* pBrush);
	void InvertRect(LPCRECT prc);
	void DrawFocusRect(LPCRECT prc);
	void FrameRect(LPCRECT prc,CBrush* pBrush);
	int SaveDC();
	BOOL RestoreDC(int iSaveDC);

	// We have to return a pointer to an object and
	// automatically dispose of it later without being
	// able to test if it is still in use ...
	// wonderful MS architecture.
	// static classes used for "original bitmap/brush/font/pen"
	CBitmap  m_bmpOrig;
	CBrush   m_brushOrig;
	CFont    m_fontOrig;
	CPen     m_penOrig;

	CBitmap* m_pBmpPrev;
	CBrush*  m_pBrushPrev;
	CFont*   m_pFontPrev;
	CPen*    m_pPenPrev;

};

#endif // CGDIOBJECT_H
