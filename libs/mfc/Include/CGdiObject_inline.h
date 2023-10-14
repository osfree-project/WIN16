/************************************************************************
  T h e   O p e n   F o u n d a t i o n    C l a s s e s
 ------------------------------------------------------------------------
  Filename   : CGdiObject_inline.h
  Version    : 0.30
  Author(s)  : William D. Herndon

 --[ Description ]-------------------------------------------------------
  This file is part of the OFC - Open Foundation Classes -.
  It is MFC compatible.
  This file is the inline implementions for the GDI object classes:
  CGdiObject, CBitmap, CBrush, CFont, CPen and CDC.
  All of these objects except CDC are based on CGdiObject, allowing
  the basic functions, such as Attach(), Detach(), DeleteObject().

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
05-30-04  0.30  wdh  Fixed ASSERT() which assigned NULL to m_hDC.

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
  Copyright (c) 2000-04 Open Foundation Classes
  Copyright (c) 2003-04 William D. Herndon
/************************************************************************/

#ifndef CGDIOBJECT_INLINE_H
#define CGDIOBJECT_INLINE_H

	// CGdiObject inlines

inline CGdiObject::CGdiObject() { m_hObject = NULL; };

inline CGdiObject::operator HGDIOBJ() const {
	return( (HGDIOBJ)m_hObject );
};

//**** Management Functions
inline BOOL CGdiObject::DeleteObject() {
	if (m_hObject == NULL)
		return( FALSE );
	::DeleteObject(Detach());
	return( TRUE );
};
inline int CGdiObject::GetObject(int nCount,LPVOID pObj) const {
	::GetObject(m_hObject, nCount, pObj);
};

inline BOOL CGdiObject::CreateStockObject(int nIndex) {
	Attach(::GetStockObject(nIndex));
	return( m_hObject != NULL );
};

	// CBitmap inlines

inline CBitmap::CBitmap() : CGdiObject() {};

inline BOOL CBitmap::CreateBitmap(int iWidth,int iHeight,UINT nPlanes,
		UINT nDepth,const void* pBits)
{
	DeleteObject();
	Attach(::CreateBitmap(iWidth, iHeight, nPlanes, nDepth, pBits));
	return( m_hObject != NULL );
};
inline BOOL CBitmap::LoadBitmap(LPCSTR pszRsc) {
	DeleteObject();
	Attach(::LoadBitmap(AfxGetResourceHandle(), pszRsc));
	return( m_hObject != NULL );
};
inline BOOL CBitmap::LoadBitmap(UINT nRscID) {
	DeleteObject();
	Attach(::LoadBitmap(AfxGetResourceHandle(), MAKEINTRESOURCE(nRscID)));
	return( m_hObject != NULL );
};
inline BOOL CBitmap::LoadOEMBitmap(UINT nRscID) {
	DeleteObject();
	Attach(::LoadBitmap(NULL, MAKEINTRESOURCE(nRscID)));
	return( m_hObject != NULL );
};
inline BOOL CBitmap::CreateBitmapIndirect(LPBITMAP pBmp) {
	DeleteObject();
	Attach(::CreateBitmapIndirect(pBmp));
	return( m_hObject != NULL );
};
inline BOOL CBitmap::CreateCompatibleBitmap(CDC* pDC,int iWidth,int iHeight) {
	DeleteObject();
	Attach(::CreateCompatibleBitmap((*pDC), iWidth, iHeight));
	return( m_hObject != NULL );
};
inline CBitmap::operator HBITMAP() const {
	return( (HBITMAP)m_hObject );
};
inline int CBitmap::GetBitmap(LPBITMAP pBmp) {
	ASSERT(m_hObject);
	return( ::GetObject(m_hObject, sizeof(BITMAP), pBmp) );
};


	// CBrush inlines

inline CBrush::CBrush() : CGdiObject() {};
inline CBrush::CBrush(COLORREF crColor) {
	 Attach(::CreateSolidBrush(crColor));
};
inline BOOL CBrush::CreateSolidBrush(COLORREF crColor) {
	DeleteObject();
	 Attach(::CreateSolidBrush(crColor));
	 return( m_hObject != NULL );
};
inline BOOL CBrush::CreateBrushIndirect(LPLOGBRUSH pBrush) {
	DeleteObject();
	 Attach(::CreateBrushIndirect(pBrush));
	 return( m_hObject != NULL );
};
inline BOOL CBrush::CreateHatchBrush(int iStyle,COLORREF crHatch) {
	DeleteObject();
	 Attach(::CreateHatchBrush(iStyle, crHatch));
	 return( m_hObject != NULL );
};
inline BOOL CBrush::CreatePatternBrush(CBitmap* pBmp) {
	DeleteObject();
	 Attach(::CreatePatternBrush(*pBmp));
	 return( m_hObject != NULL );
};
inline BOOL CBrush::CreateDIBPatternBrush(HANDLE hDIB,UINT wColSpec) {
	DeleteObject();
	 Attach(::CreateDIBPatternBrush(hDIB, wColSpec));
	 return( m_hObject != NULL );
};

	// CFont inlines

inline CFont::CFont() : CGdiObject() {};

inline BOOL CFont::CreateFontIndirect(const LOGFONT* pLogFont) {
	DeleteObject();
	Attach(::CreateFontIndirect(pLogFont));
	 return( m_hObject != NULL );
};

inline BOOL CFont::CreateFont(int iHeight,int iWidth,int iEscape,int iOrient,
	int iWeight,BYTE byItalic,BYTE byUnder,BYTE byStrike,
	BYTE byCharSet,BYTE byOutPrec,BYTE byClipPrec,BYTE byQuality,
	BYTE byPitchAndFamily,LPCSTR pszFaceName)
{
	DeleteObject();
	Attach(::CreateFont(iHeight, iWidth, iEscape, iOrient,
		iWeight, byItalic, byUnder, byStrike,
		byCharSet, byOutPrec, byClipPrec, byQuality,
		byPitchAndFamily, pszFaceName));
	 return( m_hObject != NULL );
};

	// CPen inlines

inline CPen::CPen() : CGdiObject() {};
inline CPen::CPen(int iStyle,int iWidth,COLORREF crColor) {
	Attach(::CreatePen(iStyle, iWidth, crColor));
};

inline BOOL CPen::CreatePen(int iStyle,int iWidth,COLORREF crColor) {
	DeleteObject();
	Attach(::CreatePen(iStyle, iWidth, crColor));
	return( m_hObject != NULL );
};
inline BOOL CPen::CreatePenIndirect(LPLOGPEN pPen) {
	DeleteObject();
	Attach(::CreatePenIndirect(pPen));
	return( m_hObject != NULL );
};


	// CDC inlines

#if 0 // %% ?? Implementation?
inline static	CDC* CDC::FromHandle(HDC hdc) {
	};
#endif
inline CDC::operator HDC() const { return( m_hDC ); };

//**** Management Functions

inline BOOL CDC::CreateCompatibleDC(CDC* pDC) {
	ASSERT(m_hDC == NULL);
	Attach(::CreateCompatibleDC(pDC->m_hDC));
	return( m_hDC != NULL );
};
inline BOOL CDC::CreateDC(LPCSTR pszDriver,LPCSTR pszDevice,
	LPCSTR pszOutput,LPVOID pInitData)
{
	Attach(::CreateDC(pszDriver, pszDevice,
		pszOutput, (LPDEVMODE)pInitData));
	 return( m_hDC != NULL );
};

inline BOOL CDC::CreateIC(LPCSTR pszDriver,LPCSTR pszDevice,
	LPCSTR pszOutput,LPVOID pInitData)
{
	Attach(::CreateIC(pszDriver, pszDevice,
		pszOutput, (LPDEVMODE)pInitData));
	 return( m_hDC != NULL );
};

inline BOOL CDC::DeleteDC() {
	ASSERT(m_hDC != NULL);
	return( ::DeleteDC(Detach()) );
}

//**** Attribute Functions


inline COLORREF CDC::SetTextColor(COLORREF crColor) {
	return( ::SetTextColor(m_hDC, crColor) );
};
inline COLORREF CDC::SetBkColor(COLORREF crColor) {
	return( ::SetBkColor(m_hDC, crColor) );
};
inline int CDC::SetBkMode(int iMode) { return( ::SetBkMode(m_hDC, iMode) ); };
inline int CDC::SetPolyFillMode(int iMode) {
	return( ::SetPolyFillMode(m_hDC, iMode) );
};
inline int CDC::SetROP2(int iMode) { return( ::SetROP2(m_hDC, iMode) ); };
inline int CDC::SetStretchBltMode(int iMode) {
	return( ::SetStretchBltMode(m_hDC, iMode) );
};

//**** Mapping Functions

inline int CDC::GetMapMode() const {
	return( ::GetMapMode(m_hDC) );
};
inline CPoint CDC::GetViewportOrg() const {
	CPoint ptRet;
	::GetViewportOrgEx(m_hDC, &ptRet);
	return( ptRet );
};
inline int CDC::SetMapMode(int iMode) {
	return( ::SetMapMode(m_hDC, iMode) );
};
inline CPoint CDC::SetViewportOrg(int x,int y) {
	CPoint ptRet;
	::SetViewportOrgEx(m_hDC, x, y, &ptRet);
	return( ptRet );
};
inline CPoint CDC::SetViewportOrg(POINT pt) {
	CPoint ptRet;
	::SetViewportOrgEx(m_hDC, pt.x, pt.y, &ptRet);
	return( ptRet );
};
inline CSize CDC::GetViewportExt() const {
	CSize sizRet;
	::GetViewportExtEx(m_hDC, &sizRet);
	return( sizRet );
};
inline CSize CDC::SetViewportExt(int cx,int cy) {
	CSize sizRet;
	::SetViewportExtEx(m_hDC, cx, cy, &sizRet);
	return( sizRet );
};
inline CSize CDC::SetViewportExt(SIZE sizExt) {
	CSize sizRet;
	::SetViewportExtEx(m_hDC, sizExt.cx, sizExt.cy, &sizRet);
	return( sizRet );
};
inline CPoint CDC::GetWindowOrg() const {
	CPoint ptRet;
	::GetWindowOrgEx(m_hDC, &ptRet);
	return( ptRet );
};
inline CPoint CDC::SetWindowOrg(int x,int y) {
	CPoint ptRet;
	::SetWindowOrgEx(m_hDC, x, y, &ptRet);
};
inline CPoint CDC::SetWindowOrg(POINT pt) {
	CPoint ptRet;
	::SetWindowOrgEx(m_hDC, pt.x, pt.y, &ptRet);
};
inline CSize CDC::GetWindowExt() const {
	CSize sizRet;
	::GetWindowExtEx(m_hDC, &sizRet);
	return( sizRet );
};
inline CSize CDC::SetWindowExt(int cx,int cy) {
	CSize sizRet;
	::SetWindowExtEx(m_hDC, cx, cy, &sizRet);
	return( sizRet );
};
inline CSize CDC::SetWindowExt(SIZE sizExt) {
	CSize sizRet;
	::SetWindowExtEx(m_hDC, sizExt.cx, sizExt.cy, &sizRet);
	return( sizRet );
};

inline void CDC::DPtoLP(LPPOINT pPts,int iCount) const {
	::DPtoLP(m_hDC, pPts, iCount);
};
inline void CDC::DPtoLP(LPRECT prc) const {
	::DPtoLP(m_hDC, (LPPOINT)prc, 2);
};
inline void CDC::DPtoLP(LPSIZE psiz) const {
	::DPtoLP(m_hDC, (LPPOINT)psiz, 2);
};
inline void CDC::LPtoDP(LPPOINT pPts,int iCount) const {
	::LPtoDP(m_hDC, pPts, iCount);
};
inline void CDC::LPtoDP(LPRECT prc) const {
	::LPtoDP(m_hDC, (LPPOINT)prc, 2);
};
inline void CDC::LPtoDP(LPSIZE psiz) const {
	::LPtoDP(m_hDC, (LPPOINT)psiz, 2);
};

//**** Drawing Functions

inline BOOL Arc(int x1,int y1,int x2,int y2,int x3,int y3,int x4,int y4) {
	return( ::Arc(x1, y1, x2, y2, x3, y3, x4, y4) );
};
inline BOOL Arc(LPCRECT prc,POINT ptBeg,POINT ptEnd) {
	return( ::Arc(prc->left, prc->top, prc->right, prc->bottom,
		ptBeg.x, ptBeg.y, ptEnd.x, ptEnd.y) );
};
inline BOOL CDC::BitBlt(int xDst,int yDst,int iWidth,int iHeight,CDC* pSrcDC,
	int xSrc,int ySrc,DWORD dwRop)
{
	return( ::BitBlt(m_hDC, xDst, yDst, iWidth, iHeight,
		pSrcDC->m_hDC, xSrc, ySrc, dwRop) );
};
inline BOOL CDC::Ellipse(int x1,int y1,int x2,int y2) {
	return( ::Ellipse(m_hDC, x1, y1, x2, y2) );
};
inline BOOL CDC::Ellipse(LPCRECT prc) {
	return( ::Ellipse(m_hDC, prc->left, prc->top,
		prc->right, prc->bottom) );
};
inline BOOL CDC::Pie(int x1,int y1,int x2,int y2,int x3,int y3,int x4,int y4) {
	return( ::Pie(m_hDC, x1, y1, x2, y2, x3, y3, x4, y4) );
};
inline BOOL CDC::Pie(LPCRECT prc,POINT ptBeg,POINT ptEnd) {
	return( ::Pie(m_hDC, prc->left, prc->top,
		prc->right, prc->bottom,
		ptBeg.x, ptBeg.y, ptEnd.x, ptEnd.y) );
};
inline BOOL CDC::Polygon(LPPOINT pPts,int nPts) {
	return( ::Polygon(m_hDC, pPts, nPts) );
};
inline BOOL CDC::PolyPolygon(LPPOINT pPts,LPINT pCnts,int nCnts) {
	return( ::PolyPolygon(m_hDC, pPts, pCnts, nCnts) );
};
inline BOOL CDC::Polyline(LPPOINT pPts,int nPts) {
	return( ::Polyline(m_hDC, pPts, nPts) );
};
inline BOOL CDC::PolyPolyline(LPPOINT pPts,LPDWORD pCnts,int nCnts) {
	return( ::PolyPolyline(m_hDC, pPts, pCnts, nCnts) );
};
inline BOOL CDC::Rectangle(int x1,int y1,int x2,int y2) {
	return( ::Rectangle(m_hDC, x1, y1, x2, y2) );
};
inline BOOL CDC::Rectangle(LPCRECT prc) {
	return( ::Rectangle(m_hDC, prc->left, prc->top,
		prc->right, prc->bottom) );
};
inline BOOL CDC::RoundRect(int x1,int y1,int x2,int y2,int iWidth,int iHeight)
{
	return( ::RoundRect(m_hDC, x1, y1, x2, y2, iWidth, iHeight) );
};
inline BOOL CDC::RoundRect(LPCRECT prc,POINT ptDiam) {
	return( ::RoundRect(m_hDC, prc->left, prc->top,
		prc->right, prc->bottom, ptDiam.x, ptDiam.y) );
};

//**** Text Functions

inline BOOL CDC::TextOut(int x,int y,LPCSTR pszText,int nChars)
{
	return( ::TextOut(m_hDC, x, y, pszText, nChars) );
}
inline BOOL CDC::TextOut(int x,int y,const CString& str)
{
	return( ::TextOut(m_hDC, x, y, str, str.GetLength()) );
}
inline BOOL CDC::ExtTextOut(int x,int y,UINT wFlags,LPCRECT prc,
		LPCSTR pszText,UINT nChars,LPINT pDx)
{
	return( ::ExtTextOut(m_hDC, x, y, wFlags, prc, pszText, nChars, pDx) );
}
inline BOOL CDC::ExtTextOut(int x,int y,UINT wFlags,LPCRECT prc,
		const CString& str,LPINT pDx)
{
	return( ::ExtTextOut(m_hDC, x, y, wFlags,
		prc, str, str.GetLength(), pDx) );
}
inline CSize CDC::TabbedTextOut(int x,int y,LPCSTR pszText,int nChars,
		int nTabPos,LPINT pnTabStopPos,int iTabOrigin)
{
	DWORD dwRet;
	dwRet = ::TabbedTextOut(m_hDC, x, y, pszText, nChars,
		nTabPos, pnTabStopPos, iTabOrigin);
	CSize sizRet(dwRet);
	return( sizRet );
}
inline CSize CDC::TabbedTextOut(int x,int y,const CString& str,
		int nTabPos,LPINT pnTabStopPos,int iTabOrigin)
{
	DWORD dwRet;
	dwRet = ::TabbedTextOut(m_hDC, x, y, str, str.GetLength(),
		nTabPos, pnTabStopPos, iTabOrigin);
	CSize sizRet(dwRet);
	return( sizRet );
}
inline int CDC::DrawText(LPCSTR pszText,int nChars,LPRECT prc,UINT wFlags)
{
	return( ::DrawText(m_hDC, pszText, nChars, prc, wFlags) );
}
inline int CDC::DrawText(const CString& str,LPRECT prc,UINT wFlags)
{
	return( ::DrawText(m_hDC, str, str.GetLength(), prc, wFlags) );
}


//**** Path Functions
inline BOOL CDC::AbortPath() { return( ::AbortPath(m_hDC) ); };
inline BOOL CDC::ArcTo(int x1,int y1,int x2,int y2,
	int x3,int y3,int x4,int y4)
{
	return( ::ArcTo(m_hDC, x1, y1, x2, y2, x3, y3, x4, y4) );
};
inline BOOL CDC::ArcTo(LPCRECT prc,POINT ptBeg,POINT ptEnd)
{
	return( ::ArcTo(m_hDC, prc->left, prc->top, prc->right, prc->bottom,
		ptBeg.x, ptBeg.y, ptEnd.x, ptEnd.y) );
};
inline BOOL CDC::BeginPath() { return( ::BeginPath(m_hDC) ); };
inline BOOL CDC::Chord(int x1,int y1,int x2,int y2,int x3,int y3,int x4,int y4)
{
	return( ::Chord(m_hDC, x1, y1, x2, y2, x3, y3, x4, y4) );
};
inline BOOL CDC::Chord(LPCRECT prc,POINT ptBeg,POINT ptEnd)
{
	return( ::Chord(m_hDC, prc->left, prc->top, prc->right, prc->bottom,
		ptBeg.x, ptBeg.y, ptEnd.x, ptEnd.y) );
};
inline BOOL CDC::CloseFigure() { return( ::CloseFigure(m_hDC) ); };
inline BOOL CDC::EndPath() { return( ::EndPath(m_hDC) ); };
inline BOOL CDC::FillPath() { return( ::FillPath(m_hDC) ); };
inline BOOL CDC::FlattenPath() { return( ::FlattenPath(m_hDC) ); };
inline void CDC::LineTo(int x,int y) { ::LineTo(m_hDC, x, y); };
inline void CDC::LineTo(POINT pt) { ::LineTo(m_hDC, pt.x, pt.y); };
inline void CDC::MoveTo(int x,int y) { ::MoveToEx(m_hDC, x, y, NULL); };
inline void CDC::MoveTo(POINT pt) { ::MoveToEx(m_hDC, pt.x, pt.y, NULL); };
inline BOOL CDC::PolyBezier(const POINT* pPts,int nPts) {
	return( ::PolyBezier(m_hDC, pPts, nPts) );
};
inline BOOL CDC::PolyBezierTo(const POINT* pPts,int nPts) {
	return( ::PolyBezierTo(m_hDC, pPts, nPts) );
};
inline BOOL CDC::PolyDraw(const POINT* pPts,const BYTE* pTypes,int nPts) {
	return( ::PolyDraw(m_hDC, pPts, pTypes, nPts) );
};
inline BOOL CDC::PolylineTo(LPPOINT pPts,int nPts) {
	return( ::PolylineTo(m_hDC, pPts, nPts) );
};
inline BOOL CDC::StrokeAndFillPath() { return( ::StrokeAndFillPath(m_hDC) ); };
inline BOOL CDC::StrokePath() { return( ::StrokePath(m_hDC) ); };
inline BOOL CDC::WidenPath() { return( ::WidenPath(m_hDC) ); };
inline float CDC::GetMiterLimit() const {
    float fRet;
    if ( !::GetMiterLimit(m_hDC, &fRet) )
        return( 0.0f );
    return( fRet );
};
inline BOOL CDC::SetMiterLimit(float fLimit) {
	return( ::SetMiterLimit(m_hDC, fLimit, NULL) );
};
inline int CDC::GetPath(LPPOINT pPts,LPBYTE pTypes,int nPts) const {
	return( ::GetPath(m_hDC, pPts, pTypes, nPts) );
};

//**** Info Functions

inline int  CDC::GetClipBox(LPRECT prc) {
		return( ::GetClipBox(m_hDC, prc) );
	};
inline int CDC::GetDeviceCaps(int nIndex) const {
	return( ::GetDeviceCaps(m_hDC, nIndex) );
};
inline int CDC::EnumObjects(int iObjType,
	void (CALLBACK* pfn)(LPVOID,LPARAM),LPARAM pData)
{
	return( ::EnumObjects(m_hDC, iObjType, (GOBJENUMPROC)pfn, pData) );
}
inline CSize CDC::GetTextExtent(LPCSTR pszText,int nChars) const
{
	CSize sizRet;
	::GetTextExtentPoint(m_hDC, pszText, nChars, &sizRet);
	return( sizRet );
}
inline CSize CDC::GetTextExtent(const CString& str) const
{
	CSize sizRet;
	::GetTextExtentPoint(m_hDC, str, str.GetLength(), &sizRet);
	return( sizRet );
}


//**** Miscellaneous Functions

inline void CDC::FillRect(LPCRECT prc,CBrush* pBrush) {
	::FillRect(m_hDC, prc, (HBRUSH)pBrush->GetObjHandle());
};
inline void CDC::InvertRect(LPCRECT prc) { ::InvertRect(m_hDC, prc); };
inline void CDC::DrawFocusRect(LPCRECT prc) { ::DrawFocusRect(m_hDC, prc); };
inline void CDC::FrameRect(LPCRECT prc,CBrush* pBrush) {
	::FrameRect(m_hDC, prc, (HBRUSH)pBrush->GetObjHandle());
};
inline int CDC::SaveDC() { return( ::SaveDC(m_hDC) ); };
inline BOOL CDC::RestoreDC(int iSaveDC) {
	return( ::RestoreDC(m_hDC, iSaveDC) );
};

#endif // CGDIOBJECT_INLINE_H
