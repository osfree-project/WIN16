#include "user.h"

INTWNDCLASS far * GetClassPtr(LPCSTR lpszClassName, HANDLE hInstance, BOOL checkGlobalClasses)
{
  // @todo Write me!!
}

/***********************************************************************
 *		RegisterClass (USER.57)
 */
ATOM WINAPI RegisterClass( const WNDCLASS far *wc )
{
    INTWNDCLASS far * intWndCls;
    BOOL isGlobalClass;
    WORD expWinVersion;
    LPSTR menuName;

    intWndCls=GetClassPtr(wc->lpszClassName, wc->hInstance, FALSE);

    if (intWndCls)
    {
      return 0;
    }

    isGlobalClass = (wc->style & CS_GLOBALCLASS)?TRUE:FALSE;

    expWinVer=GetExpWinVer(wc->hInstance);

    if (expWinVer < 0x0300) isGlobalClass=TRUE;

    if (isGlobalClass)
    {
      intWndCls=GetClassPtrAsm(FindAtom(wc->lpszClassName), GetModuleHandle(MAKELP(0, wc->hInstance)), TRUE);
      if (intWndCls) return 0;
    }

    intWndCls=UserLocalAlloc(LT_USER_CLASS, LMEM_ZEROINIT, sizeof(INTWNDCLASS)+lpWndClass->cbClsExtra);
    if (!intWndClass) return 0;
   
    LCopyStruct(lpWndCls, intWndCls+0xa,sizeof (WNDCLASS));

   if (intWndCls->hIcon == HIconSample) intWndCls->hIcon = HIconWindows;

   if (isGlobalCls) intWndCls->style = intWndCls->style | GS_GLOBALCLASS;
 
   intWndClass -> atomCls = AddAtom (intWndCls->lpszClassName);

   if (!intWndClass->atomCls) goto RegisterClassNoMem;

   intWndCls->lpszClassName=NULL;

   
    return 0;
}

