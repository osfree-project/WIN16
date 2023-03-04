#include <string.h>
#include <ctype.h>

#include <windows.h>

#include "user.h"

#define GlobalPtrHandle(lp) \
  ((HGLOBAL)LOWORD(GlobalHandle(SELECTOROF(lp))))

#define     GlobalUnlockPtr(lp)      \
                GlobalUnlock(GlobalPtrHandle(lp))

#define GlobalFreePtr(lp) \
  (GlobalUnlockPtr(lp),(BOOL)GlobalFree(GlobalPtrHandle(lp)))

#define GlobalAllocPtr(flags, cb) \
  (GlobalLock(GlobalAlloc((flags), (cb))))

/* MSWin dialog class name */
#define	TWIN_DIALOGCLASS "#32770"

//@todo It seems SYSTEMGLOBAL class not required for win16...
WNDPROC lpfnDefaultBinToNat = NULL;
WNDPROC lpfnDefaultNatToBin = NULL;

typedef unsigned long ATOMID;

typedef struct {
	ATOMID	q;		/* what a string 'hashes' to 	*/
	long   	idx;		/* index into data table	*/
	long	refcnt;		/* how many clients have it 	*/
	long	idsize;		/* space used by this slot	*/
} ATOMENTRY;
typedef ATOMENTRY far *LPATOMENTRY;

typedef struct {
	ATOMENTRY far	*AtomTable;	/* pointer to table data 	*/
	char	far	*AtomData;	/* pointer to name data 	*/
	unsigned long	 TableSize;	/* number items in this table   */
	unsigned long	 DataSize;	/* space used by string data    */
	LPVOID		lpDrvData;
} ATOMTABLE;
typedef ATOMTABLE far *LPATOMTABLE;

#define GLOBALAddAtomEx AddAtomEx
#define GLOBALDeleteAtomEx DeleteAtomEx
#define GLOBALGetAtomNameEx GetAtomNameEx

#define ATOMBASE	 0xcc00

static ATOMID
AtomHashString(LPCSTR lp,int far *lplen)
{
	ATOMID 	q;
	char  far *p,ch;
	int	len;

	/* if we have an intatom... */
	if(HIWORD(lp) == 0) {
		if(lplen) *lplen = 0;
		return (ATOMID)lp;
	}

	/* convert the string to an internal representation */
	for(p=(LPSTR)lp,q=0,len=0;(ch=*p++);len++)
		q = (q<<1) + islower(ch)?toupper(ch):ch;

	/* 0 is reserved for empty slots */
	if(q == 0)
		q++;

	/* avoid strlen later */
	if(lplen) {
		*lplen = ++len;
	}
	return q;
}


/********************************************************/
/*	convert an atom index into a pointer into an 	*/
/* 	atom table.  This validates the pointer is in   */
/*	range, and that the data is accessible		*/
/********************************************************/

static ATOMENTRY far *
GetAtomPointer(ATOMTABLE far *at,int index)
{
	ATOMENTRY far *lp;
	
	/* if no table, then no pointers */
	if(at->AtomTable == 0)
		return 0;

	/* bad index */
	if((index < 0) || (index >= at->TableSize))
		return 0;

	/* we have a pointer */
	lp = &at->AtomTable[index];

	/* is the index past stored data, validity check		*/
	/* LATER: is the size of the entry within the available space 	*/
	if(lp->idx > at->DataSize)
		return 0;

	return lp;
}

ATOM
DeleteAtomEx(ATOMTABLE far *at,ATOM atom)
{
	ATOMENTRY far *lp;
	
	/* a free slot has q == 0 && refcnt == 0 */
	if((lp = GetAtomPointer(at,atom - ATOMBASE))) {
		if(lp->idsize)
			lp->refcnt--;

		if(lp->refcnt == 0) {
			return lp->q = 0;
		}
	}
	return atom;
}

UINT
GetAtomNameEx(ATOMTABLE far *at,ATOM atom,LPSTR lpstr,int len)
{
	ATOMENTRY far *lp;
	char 	 far  *atomstr;
	int	   atomlen;
	
	/* return the atom name, or create the INTATOM */
	if((lp = GetAtomPointer(at,atom - ATOMBASE))) {
		if(lp->idsize) {
			atomlen = lstrlen(atomstr = &at->AtomData[lp->idx]);
			if (atomlen < len)
			    lstrcpy(lpstr,atomstr);
			else {
			    lstrcpyn(lpstr,atomstr,len-1);
			    lpstr[len-1] = '\0';
			}
			return (UINT)lstrlen(lpstr);
		} else {
			wsprintf(lpstr,"#%d",lp->q);
			return (UINT)lstrlen(lpstr);
		}
	}
	return 0;
}

ATOM
FindAtomEx(ATOMTABLE far *at,LPCSTR lpstr)
{
	ATOMID		q;
	LPATOMENTRY   	lp;
	int		index;
	int		atomlen;

	/* convert string to 'q', and get length */
	q = AtomHashString(lpstr,&atomlen);

	/* find the q value, note: this could be INTATOM */
	/* if q matches, then do case insensitive compare*/
	for(index = 0;(lp = GetAtomPointer(at,index));index++) {
		if(lp->q == q) {	
			if(HIWORD(lpstr) == 0)
				return ATOMBASE + index;
			if(_fstricmp(&at->AtomData[lp->idx],lpstr) == 0)
				return ATOMBASE + index;
		}
	}
	return 0;
}

ATOM
AddAtomEx(ATOMTABLE far *at,LPCSTR lpstr)
{
	ATOM atom;
	ATOMID		q;
	LPATOMENTRY   	lp,lpfree;
	int		index,freeindex;
	int		atomlen;
	int		newlen;
	
	/* if we already have it, bump refcnt */
	if((atom = FindAtomEx(at,lpstr))) {
		lp = GetAtomPointer(at,atom - ATOMBASE);
		if(lp->idsize) lp->refcnt++;
		return atom;
	}

	/* add to a free slot */
	q = AtomHashString(lpstr,&atomlen);

	lpfree 	  = 0;
	freeindex = 0;

	for(index = 0;(lp = GetAtomPointer(at,index));index++) {
		if(lp->q == 0 && lp->refcnt == 0) {	
			if(lp->idsize > atomlen) {
				if ((lpfree == 0) ||
					    (lpfree->idsize > lp->idsize)) {
					lpfree = lp;
					freeindex = index;
				}
			}
		}
	}
	/* intatoms do not take space in data, but do get new entries */
	/* an INTATOM will have length of 0 			      */
	if(lpfree && atomlen) {
		lpfree->q = q;
		lpfree->refcnt = 1;			
		lstrcpyn(&at->AtomData[lpfree->idx],lpstr,atomlen);
		return freeindex + ATOMBASE;
	}

	/* no space was available, or we have an INTATOM		*/
	/* so expand or create the table 				*/
	if(at->AtomTable == 0) {
		at->AtomTable = (ATOMENTRY far *) GlobalAllocPtr(GPTR, sizeof(ATOMENTRY));
		at->TableSize = 1;
		lp = at->AtomTable;
		index = 0;
	} else {
		at->TableSize++;
		at->AtomTable = (ATOMENTRY far *) GlobalLock(GlobalReAlloc(GlobalPtrHandle(
			(char far *) at->AtomTable),
			at->TableSize * sizeof(ATOMENTRY), GMEM_MOVEABLE));
		lp = &at->AtomTable[at->TableSize - 1];
	}

	/* set in the entry */
	lp->refcnt = 1;
	lp->q      = q;
	lp->idsize = atomlen;
	lp->idx    = 0;

	/* add an entry if not intatom... */
	if(atomlen) {
		newlen = at->DataSize + atomlen;

		if(at->AtomData == 0) {
			at->AtomData = (char far *) GlobalAllocPtr(GPTR, newlen);
			lp->idx = 0;
		} else {
			at->AtomData = (char far *) GlobalLock(GlobalReAlloc(GlobalPtrHandle(at->AtomData),newlen, GMEM_MOVEABLE));
			lp->idx = at->DataSize;
		}

		lstrcpy(&at->AtomData[lp->idx],lpstr);
		at->DataSize = newlen;
	}	

	return index + ATOMBASE;
}

#if 0
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

#endif

static LPCLASSINFO lpClasses[] = {
	NULL,
	NULL,
	NULL,
	NULL,
};

static ATOMTABLE ClassTable;


static LPCLASSINFO
SearchClass(LPCLASSINFO lpClassType, LPCSTR lpClassStr,
			HMODULE hModule)
{
    LPCLASSINFO lpClass;
    char lpAtomString[80];
    char lpDialogString[80];
    LPSTR lpClassName;

    if (!hModule && !HIWORD((DWORD)lpClassStr) &&
	LOWORD((DWORD)lpClassStr) == LOWORD((DWORD)WC_DIALOG)) {
	lstrcpy(lpDialogString,TWIN_DIALOGCLASS);
	lpClassName = lpDialogString;
    }
    else
	lpClassName = (LPSTR)lpClassStr;

    for(lpClass = lpClassType;lpClass;lpClass = lpClass->lpClassNext) {
	if (!HIWORD((DWORD)lpClassName)) {
	    if (lpClass->atmClassName ==
		(ATOM)(LOWORD((DWORD)lpClassName)) &&
		(!hModule || (lpClass->hModule == hModule)))
		return lpClass;
	}
	else {
	    if (lpClass->style & CS_GLOBALCLASS)
		GLOBALGetAtomNameEx(&ClassTable,
			lpClass->atmClassName,lpAtomString,80);
	    else
		GetAtomNameEx(&ClassTable,
			lpClass->atmClassName,lpAtomString,80);
	    if((!_fstricmp(lpAtomString,lpClassName)) &&
		(!hModule || (lpClass->hModule == hModule)))
		return lpClass;
	}
    }
    return 0;
}

LPCLASSINFO
FindClass(LPCSTR lpClassName, HINSTANCE hInstance)
{
    LPCLASSINFO lpClassFound;
    HMODULE hModule;

//    APISTR((LF_APICALL, "FindClass(LPCSTR=%p,HINSTANCE=%x)\n",
//	HIWORD(lpClassName) ? lpClassName : "atom", hInstance));

    if (hInstance)
//	hModule = GetModuleFromInstance(hInstance);
        hModule = LOWORD(GetModuleHandle(MAKELP(0, hInstance)));
    else
	hModule = 0;
	lpClassFound =	SearchClass(lpClasses[APPLOCAL], lpClassName, hModule);

    if (lpClassFound) {
//    	APISTR((LF_APIRET,"FindClass: returns HCLASS32 %d\n",lpClassFound));
	return lpClassFound;
    }
		
	lpClassFound =	SearchClass(lpClasses[APPGLOBAL],lpClassName,(HMODULE)0);
    if (lpClassFound) {
//    	APISTR((LF_APIRET,"FindClass: returns HCLASS32 %d\n",lpClassFound));
        return lpClassFound;
    }

    lpClassFound =  SearchClass(lpClasses[SYSGLOBAL], lpClassName,(HMODULE)0);
//    APISTR((LF_APIRET,"FindClass: returns HCLASS32 %d\n",lpClassFound));
    return lpClassFound;
}

static LPCLASSINFO
InternalRegisterClassEx(const WNDCLASSEX far *lpwcx)
{
    int Type;
    HMODULE hModule;
    LPCLASSINFO lpNewC, far *lpClassType;
    int    size;

    if (lpwcx->hInstance) {
//        if (!(hModule = GetModuleFromInstance(lpwcx->hInstance))) {
        if (!(hModule = LOWORD(GetModuleHandle(MAKELP(0, lpwcx->hInstance))))) {

	    return 0;
        }
    }
    else
	hModule = 0;

    if (!lpwcx->lpszClassName)
	return 0;

    if (lpwcx->style & CS_SYSTEMGLOBAL)
	Type = SYSGLOBAL;
    else
	Type = (lpwcx->style & CS_GLOBALCLASS)?APPGLOBAL:APPLOCAL;
    lpClassType = &lpClasses[Type];

    if (*lpClassType) {
    	lpNewC = SearchClass(*lpClassType, lpwcx->lpszClassName, hModule);
        if (lpNewC) {
            return 0;
        }
    }

    lpNewC = (LPCLASSINFO)GlobalAllocPtr(GPTR, sizeof(CLASSINFO));

    _fmemset((LPSTR)lpNewC, 0, sizeof(CLASSINFO));

    if (lpwcx->cbClsExtra)
	lpNewC->lpClsExtra = (LPSTR)GlobalAllocPtr(GPTR, lpwcx->cbClsExtra);
    else
	lpNewC->lpClsExtra = NULL;

    lpNewC->wClassType = Type;
    lpNewC->style = lpwcx->style;
    lpNewC->lpfnWndProc = lpwcx->lpfnWndProc;
    lpNewC->cbClsExtra = lpwcx->cbClsExtra;
    lpNewC->cbWndExtra = lpwcx->cbWndExtra;
    lpNewC->hModule = hModule;
    lpNewC->hIcon = lpwcx->hIcon;
    lpNewC->hCursor = lpwcx->hCursor;
    lpNewC->hbrBackground = lpwcx->hbrBackground;
    lpNewC->nUseCount = 0;
    lpNewC->hIconSm = lpwcx->hIconSm;
    if (!(lpwcx->style & CS_SYSTEMGLOBAL)) {
	lpNewC->lpfnNatToBin = lpfnDefaultNatToBin;
	lpNewC->lpfnBinToNat = lpfnDefaultBinToNat;
    }
    if (HIWORD((DWORD)(lpwcx->lpszMenuName))) {
	size = lstrlen(lpwcx->lpszMenuName)+1;
	lpNewC->lpMenuName = GlobalAllocPtr(GPTR, size);
	lstrcpy(lpNewC->lpMenuName, lpwcx->lpszMenuName);
    } else
	lpNewC->lpMenuName = (LPSTR)lpwcx->lpszMenuName;

    if (!HIWORD((DWORD)(lpwcx->lpszClassName)))
	lpNewC->atmClassName =
	    (ATOM)(LOWORD((DWORD)lpwcx->lpszClassName));
    else
	lpNewC->atmClassName = (lpwcx->style & CS_GLOBALCLASS)?
	    GLOBALAddAtomEx(&ClassTable,lpwcx->lpszClassName):
	    AddAtomEx(&ClassTable,lpwcx->lpszClassName);

    if (lpNewC->cbClsExtra)
	_fmemset(lpNewC->lpClsExtra, 0, lpNewC->cbClsExtra);

    /* link in the new class */
    lpNewC->lpClassNext = *lpClassType;
    if (*lpClassType) (*lpClassType)->lpClassPrev = lpNewC;
    *lpClassType = lpNewC;

    return lpNewC;
}

/***********************************************************************
 *		RegisterClassEx (USER.397)
 */
ATOM WINAPI
RegisterClassEx(const WNDCLASSEX far *lpwcx)
{
    LPCLASSINFO lpClassInfo;
    ATOM atmClass;

//    APISTR((LF_APICALL, "RegisterClassEx(WNDCLASS *=%x)\n", lpwcx));
    lpClassInfo = InternalRegisterClassEx(lpwcx);
    atmClass = (lpClassInfo)?lpClassInfo->atmClassName:(ATOM)0;
//    APISTR((LF_APIRET, "RegisterClassEx: returns ATOM %x\n",atmClass));
    return atmClass;

}


/***********************************************************************
 *		RegisterClass (USER.57)
 */
ATOM WINAPI
RegisterClass(const WNDCLASS far *lpwc)
{
    WNDCLASSEX wcx;
    ATOM atom;

//    APISTR((LF_APICALL, "RegisterClass(WNDCLASS *=%x)\n", lpwc));

    wcx.cbSize = sizeof(WNDCLASSEX);
    wcx.style = lpwc->style;
    wcx.lpfnWndProc = lpwc->lpfnWndProc;
    wcx.cbClsExtra = lpwc->cbClsExtra;
    wcx.cbWndExtra = lpwc->cbWndExtra;
    wcx.hInstance = lpwc->hInstance;
    wcx.hIcon = lpwc->hIcon;
    wcx.hCursor = lpwc->hCursor;
    wcx.hbrBackground = lpwc->hbrBackground;
    wcx.lpszMenuName = lpwc->lpszMenuName;
    wcx.lpszClassName = lpwc->lpszClassName;
    wcx.hIconSm = (HICON)0;

    atom = RegisterClassEx(&wcx);
//    APISTR((LF_APIRET, "RegisterClass: returns ATOM %x\n",atom));
    return atom;

}

BOOL TWIN_InternalUnregisterClass(LPCLASSINFO lpClassFound)
{
    if (lpClassFound->lpClsExtra)
	GlobalFreePtr(lpClassFound->lpClsExtra);

    if (HIWORD((DWORD)(lpClassFound->lpMenuName)))
	GlobalFreePtr(lpClassFound->lpMenuName);

    if (lpClassFound->style & CS_GLOBALCLASS)
	GLOBALDeleteAtomEx(&ClassTable,lpClassFound->atmClassName);
    else
	DeleteAtomEx(&ClassTable,lpClassFound->atmClassName);

    if (lpClassFound->lpClassPrev)
	lpClassFound->lpClassPrev->lpClassNext = lpClassFound->lpClassNext;
    if (lpClassFound->lpClassNext)
	lpClassFound->lpClassNext->lpClassPrev = lpClassFound->lpClassPrev;
    if (lpClasses[lpClassFound->wClassType] == lpClassFound)
	lpClasses[lpClassFound->wClassType] = lpClassFound->lpClassNext;

    GlobalFreePtr((LPSTR)lpClassFound);

    return TRUE;
}

/***********************************************************************
 *		UnregisterClass (USER.403)
 */
BOOL WINAPI
UnregisterClass(LPCSTR lpClassName, HINSTANCE hInstance)
{
    LPCLASSINFO lpClassFound;
    BOOL rc;

//    APISTR((LF_APICALL,"UnregisterClass(LPCSTR=%s, HINSTANCE=%x)\n",
//		HIWORD(lpClassName)?lpClassName:"ATOM", 
//		hInstance));

    if (!(lpClassFound = FindClass(lpClassName, hInstance))) {
//    	APISTR((LF_APIFAIL,"UnregisterClass: returns BOOL FALSE\n"));
	return FALSE;
    }

    if ((lpClassFound->wClassType == SYSGLOBAL) ||
	(lpClassFound->nUseCount)) {
//    	APISTR((LF_APIFAIL,"UnregisterClass: returns BOOL FALSE\n"));
	return FALSE;
    }


    rc =  TWIN_InternalUnregisterClass(lpClassFound);

//    APISTR((LF_APIRET,"UnregisterClass: returns BOOL %d\n",rc));
    return rc;
}

