#include <windows.h>

// Selector of Global Atom Table
static WORD GlobalAtomTable_Selector;

/* This function sets current DS value */
extern  void          SetDS( unsigned short );
#pragma aux SetDS               = \
        "mov    ds,ax"          \
        parm                   [ax];

#define  SetGlobalTableDS() if (GlobalAtomTable_Selector) SetDS(GlobalAtomTable_Selector)

// This is initialization of Global Atoms. This function must be called during USER.EXE initialization.
void __loadds GlobalInitAtom(void)
{
  GlobalAtomTable_Selector=GlobalAlloc(GMEM_MOVEABLE | GMEM_ZEROINIT | GMEM_DDESHARE, 0xfa);
  if (GlobalAtomTable_Selector)
  {
    GlobalAtomTable_Selector=SELECTOROF(GlobalLock(GlobalAtomTable_Selector));
    SetGlobalTableDS();
    LocalInit(0, 0, 0xea);
    InitAtomTable(0x25);
    GlobalUnlock(GlobalAtomTable_Selector);
  }
}

/***********************************************************************
 *		GlobalAddAtom (USER.268)
 */
ATOM WINAPI
GlobalAddAtom(LPCSTR lpstr)
{
  SetGlobalTableDS();
  return AddAtom(lpstr);
}

/***********************************************************************
 *		GlobalFindAtom (USER.270)
 */
ATOM WINAPI
GlobalFindAtom(LPCSTR lpstr)
{
  SetGlobalTableDS();
  return FindAtom(lpstr);
}

/***********************************************************************
 *		GlobalGetAtomName (USER.271)
 */
UINT WINAPI
GlobalGetAtomName(ATOM atom,LPSTR lpszbuf,int len)
{
  SetGlobalTableDS();
  return GetAtomName(atom, lpszbuf, len);
}

/***********************************************************************
 *		GlobalDeleteAtom (USER.269)
 */
ATOM WINAPI
GlobalDeleteAtom(ATOM atom)
{
  SetGlobalTableDS();
  return DeleteAtom(atom);
}
