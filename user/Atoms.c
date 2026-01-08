#include <user.h>

#define  SetGlobalTableDS() SetDS(GlobalAtomTable_Selector)

// This is initialization of Global Atoms. This function must be called during USER.EXE initialization.
// See Windows Internals p.68
VOID WINAPI GlobalInitAtom(void)
{
	PushDS();
	FUNCTION_START
	GlobalAtomTable_Selector=GlobalAlloc(GMEM_MOVEABLE | GMEM_ZEROINIT | GMEM_DDESHARE, 0xfa);
	if (GlobalAtomTable_Selector)
	{
		GlobalAtomTable_Selector=SELECTOROF(GlobalLock(GlobalAtomTable_Selector));
		SetGlobalTableDS();
		LocalInit(0, 0, 0xea);
		InitAtomTable(0x25);
		GlobalUnlock(GlobalAtomTable_Selector);
	}
	FUNCTION_END
	PopDS();
}

/***********************************************************************
 *		GlobalAddAtom (USER.268)
 */
ATOM WINAPI GlobalAddAtom(LPCSTR lpstr)
{
	ATOM res;
	PushDS();
	FUNCTION_START
	SetGlobalTableDS();
	res=AddAtom(lpstr);
	FUNCTION_END
	PopDS();
	return res;
}

/***********************************************************************
 *		GlobalDeleteAtom (USER.269)
 */
ATOM WINAPI GlobalDeleteAtom(ATOM atom)
{
	ATOM res;
	PushDS();
	FUNCTION_START
	SetGlobalTableDS();
	res=DeleteAtom(atom);
	FUNCTION_END
	PopDS();
	return res;
}

/***********************************************************************
 *		GlobalFindAtom (USER.270)
 */
ATOM WINAPI GlobalFindAtom(LPCSTR lpstr)
{
	ATOM res;
	PushDS();
	FUNCTION_START
	SetGlobalTableDS();
	res=FindAtom(lpstr);
	FUNCTION_END
	PopDS();
	return res;
}

/***********************************************************************
 *		GlobalGetAtomName (USER.271)
 */
UINT WINAPI GlobalGetAtomName(ATOM atom,LPSTR lpszbuf,int len)
{
	UINT res;
	PushDS();
	FUNCTION_START
	SetGlobalTableDS();
	res=GetAtomName(atom, lpszbuf, len);
	FUNCTION_END
	PopDS();
	return res;
}

