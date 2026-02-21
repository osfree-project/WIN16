/*    
    	Copyright 2026 Yuri Prokushev

This library is free software; you can redistribute it and/or
modify it under the terms of the GNU Lesser General Public
License as published by the Free Software Foundation; either
version 2.1 of the License, or (at your option) any later version.

This library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public
License along with this library; if not, see
<https://www.gnu.org/licenses/>.

*/

#include <user.h>

// Global Atom Table reuses standard Atom Table functions which work with local heap (uses DS).
// We just switch DS to Global Atom Heap selector, do things and switch DS back.

// This is initialization of Global Atoms. This function must be called during USER.EXE initialization.
// See Windows Internals p.68
//
// Here we allocate Global Atom Table heap, switch DS to it, initialize local heap and init atom table
// After all done - switch DS back
VOID WINAPI GlobalInitAtom(void)
{
	PushDS();
	FUNCTION_START

	// Allocate memory for Global Atom Table Heap
	GlobalAtomTable_Selector=GlobalAlloc(GMEM_MOVEABLE | GMEM_ZEROINIT | GMEM_DDESHARE, 0xfa);

	if (GlobalAtomTable_Selector)
	{
		GlobalAtomTable_Selector=SELECTOROF(GlobalLock(GlobalAtomTable_Selector));
		SetGlobalTableDS();
		LocalInit(0, 0, 0xea);
		InitAtomTable(0x25);
		//GlobalUnlock(GlobalAtomTable_Selector);
	}

	FUNCTION_END
	PopDS();
}

/***********************************************************************
 *		GlobalAddAtom (USER.268)
 *
 * Switch DS to Global Atom Table heap, call AddAtom, after all done - switch DS back
 */
ATOM WINAPI GlobalAddAtom(LPCSTR lpstr)
{
	ATOM res;

	PushDS();
	FUNCTION_START

	if (HIWORD(lpstr))
	{
		TRACE("'%S' ", lpstr);
	}
	else
	{
		TRACE("%04x ", LOWORD(lpstr) );
	}

	SetGlobalTableDS();
	res=AddAtom(lpstr);

	FUNCTION_END
	PopDS();
	return res;
}

/***********************************************************************
 *		GlobalDeleteAtom (USER.269)
 *
 * Switch DS to Global Atom Table heap, call DeleteAtom, after all done - switch DS back
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
 *
 * Switch DS to Global Atom Table heap, call FindAtom, after all done - switch DS back
 */
ATOM WINAPI GlobalFindAtom(LPCSTR lpstr)
{
	ATOM res;

	PushDS();
	FUNCTION_START

	if (HIWORD(lpstr))
	{
		TRACE("'%S' ", lpstr);
	}
	else
	{
		TRACE("%04x ", LOWORD(lpstr) );
	}

	SetGlobalTableDS();
	res=FindAtom(lpstr);

	FUNCTION_END
	PopDS();
	return res;
}

/***********************************************************************
 *		GlobalGetAtomName (USER.271)
 *
 * Switch DS to Global Atom Table heap, call GetAtomName, after all done - switch DS back
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

