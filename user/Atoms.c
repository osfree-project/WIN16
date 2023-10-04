#include <user.h>

// Selector of Global Atom Table
static WORD GlobalAtomTable_Selector;

extern  void PushDS( void );
#pragma aux PushDS = "push ds";

extern  void PopDS( void );
#pragma aux PopDS = "pop ds";

/* This function sets current DS value */
extern  void          SetDS( unsigned short );
#pragma aux SetDS               = \
        "mov    ds,ax"          \
        parm                   [ax];

#define  SetGlobalTableDS() if (GlobalAtomTable_Selector) SetDS(GlobalAtomTable_Selector)

// This is initialization of Global Atoms. This function must be called during USER.EXE initialization.
void PASCAL GlobalInitAtom(void)
{
	FUNCTION_START
  PushDS();
  GlobalAtomTable_Selector=GlobalAlloc(GMEM_MOVEABLE | GMEM_ZEROINIT | GMEM_DDESHARE, 0xfa);
  if (GlobalAtomTable_Selector)
  {
    GlobalAtomTable_Selector=SELECTOROF(GlobalLock(GlobalAtomTable_Selector));
    SetGlobalTableDS();
    LocalInit(0, 0, 0xea);
    InitAtomTable(0x25);
    GlobalUnlock(GlobalAtomTable_Selector);
  }
  PopDS();
}

/***********************************************************************
 *		GlobalAddAtom (USER.268)
 */
ATOM WINAPI
GlobalAddAtom(LPCSTR lpstr)
{
	FUNCTION_START
  SetGlobalTableDS();
  return AddAtom(lpstr);
}

/***********************************************************************
 *		GlobalFindAtom (USER.270)
 */
ATOM WINAPI
GlobalFindAtom(LPCSTR lpstr)
{
	FUNCTION_START
  SetGlobalTableDS();
  return FindAtom(lpstr);
}

/***********************************************************************
 *		GlobalGetAtomName (USER.271)
 */
UINT WINAPI
GlobalGetAtomName(ATOM atom,LPSTR lpszbuf,int len)
{
	FUNCTION_START
  SetGlobalTableDS();
  return GetAtomName(atom, lpszbuf, len);
}

/***********************************************************************
 *		GlobalDeleteAtom (USER.269)
 */
ATOM WINAPI
GlobalDeleteAtom(ATOM atom)
{
	FUNCTION_START
  SetGlobalTableDS();
  return DeleteAtom(atom);
}
