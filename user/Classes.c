/*
 * Window classes functions
 *
 * Copyright 1993 Alexandre Julliard
 */

//#include <stdlib.h>
//#include <stdio.h>
//#include <string.h>
#include "class.h"
//#include "user.h"
//#include "win.h"
#include "dce.h"
//#include "atom.h"
//#include "ldt.h"
//#include "toolhelp.h"
//#include "stddebug.h"
/* #define DEBUG_CLASS */
//#include "debug.h"


static HCLASS firstClass = 0;

#ifdef DEBUG
/***********************************************************************
 *           CLASS_DumpClass
 *
 * Dump the content of a class structure to stderr.
 */
void CLASS_DumpClass( HCLASS hClass )
{
    CLASS *ptr;
    char className[80];
    int i;

    if (!(ptr = CLASS_FindClassPtr( hClass )))
    {
        TRACE("%04x is not a class handle\n", hClass );
        return;
    }
    GlobalGetAtomName( ptr->atomName, className, sizeof(className) );

    TRACE("Class %04x:\n", hClass );
    TRACE(
             "next=%04x  name=%04x '%s'  style=%04x  wndProc=%08lx\n"
             "inst=%04x  hdce=%04x  icon=%04x  cursor=%04x  bkgnd=%04x\n"
             "clsExtra=%d  winExtra=%d  #windows=%d\n",
             ptr->hNext, ptr->atomName, className, ptr->wc.style,
             (DWORD)ptr->wc.lpfnWndProc, ptr->wc.hInstance, ptr->hdce,
             ptr->wc.hIcon, ptr->wc.hCursor, ptr->wc.hbrBackground,
             ptr->wc.cbClsExtra, ptr->wc.cbWndExtra, ptr->cWindows );
    if (ptr->wc.cbClsExtra)
    {
        TRACE("extra bytes:" );
        for (i = 0; i < ptr->wc.cbClsExtra; i++)
            TRACE(" %02x", *((BYTE *)ptr->wExtra+i) );
        //fprintf( stderr, "\n" );
    }
//    fprintf( stderr, "\n" );
}


/***********************************************************************
 *           CLASS_WalkClasses
 *
 * Walk the class list and print each class on stderr.
 */
void CLASS_WalkClasses(void)
{
    HCLASS hClass = firstClass;
    CLASS *ptr;
    char className[80];

    TRACE("Class  Name                Style WndProc\n" );
    while (hClass)
    {
        if (!(ptr = CLASS_FindClassPtr( hClass )))
        {
            TRACE("*** Bad class %x in list\n", hClass );
            return;
        }
        GlobalGetAtomName( ptr->atomName, className, sizeof(className) );
        TRACE("%x  %S %x %P\n",
                 hClass, className, ptr->wc.style, ptr->wc.lpfnWndProc);
        hClass = ptr->hNext;
    }
}
#endif

/***********************************************************************
 *           CLASS_FindClassByName
 *
 * Return a handle and a pointer to the class.
 * 'ptr' can be NULL if the pointer is not needed.
 */
HCLASS CLASS_FindClassByName( LPCSTR name, HINSTANCE hinstance, CLASS **ptr )
{
    ATOM atom;
    HCLASS class;
    CLASS * classPtr;

    if (!(atom = GlobalFindAtom( name ))) return 0;

      /* First search task-specific classes */

    for (class = firstClass; (class); class = classPtr->hNext)
    {
        classPtr = (CLASS *) LocalLock(class);
        if (classPtr->wc.style & CS_GLOBALCLASS) continue;
        if ((classPtr->atomName == atom) && 
            ( (hinstance==(HINSTANCE)0xffff) ||
	      (hinstance == classPtr->wc.hInstance) ) )
        {
            if (ptr) *ptr = classPtr;
            return class;
        }
    }
    
      /* Then search global classes */

    for (class = firstClass; (class); class = classPtr->hNext)
    {
        classPtr = (CLASS *) LocalLock(class);
        if (!(classPtr->wc.style & CS_GLOBALCLASS)) continue;
        if (classPtr->atomName == atom)
        {
            if (ptr) *ptr = classPtr;
            return class;
        }
    }

    return 0;
}


/***********************************************************************
 *           CLASS_FindClassPtr
 *
 * Return a pointer to the CLASS structure corresponding to a HCLASS.
 */
CLASS * CLASS_FindClassPtr( HCLASS hclass )
{
    CLASS * ptr;
    
    if (!hclass) return NULL;
    ptr = (CLASS *) LocalLock( hclass );
    if (ptr->wMagic != CLASS_MAGIC) return NULL;
    return ptr;
}


/***********************************************************************
 *           RegisterClass    (USER.57)
 */
ATOM WINAPI RegisterClass(const WNDCLASS FAR * class )
{
    CLASS * newClass, * prevClassPtr;
    HCLASS handle, prevClass;
    int classExtra;
	HMODULE hModule;
	FUNCTION_START
    TRACE("RegisterClass: wndproc=%x:%x hinst=%x name='%S' background %x",
                 class->lpfnWndProc, class->hInstance,
                 HIWORD(class->lpszClassName) ?
                  (class->lpszClassName) : "(int)",
                 class->hbrBackground );
    TRACE("               style=%x clsExtra=%d winExtra=%d",
                  class->style, class->cbClsExtra, class->cbWndExtra );
    
      /* Window classes are owned by modules, not instances */
//    class->hInstance = GetExePtr( class->hInstance );
	hModule = GetExePtr( class->hInstance );
    

      /* Check if a class with this name already exists */
    prevClass = CLASS_FindClassByName( class->lpszClassName,
                                       hModule/*class->hInstance*/, &prevClassPtr );
    if (prevClass)
    {
	  /* Class can be created only if it is local and */
	  /* if the class with the same name is global.   */

	if (class->style & CS_GLOBALCLASS) return 0;
	if (!(prevClassPtr->wc.style & CS_GLOBALCLASS)) return 0;
    }

      /* Create class */

    classExtra = (class->cbClsExtra < 0) ? 0 : class->cbClsExtra;
    handle = LocalAlloc (LMEM_FIXED, sizeof(CLASS) + classExtra );
    if (!handle) return 0;
    newClass = (CLASS *) LocalLock( handle );
    newClass->hNext         = firstClass;
    newClass->wMagic        = CLASS_MAGIC;
    newClass->cWindows      = 0;  
    newClass->wc            = *class;
    newClass->wc.cbWndExtra = (class->cbWndExtra < 0) ? 0 : class->cbWndExtra;
    newClass->wc.cbClsExtra = classExtra;

    newClass->atomName = GlobalAddAtom( class->lpszClassName );
    newClass->wc.lpszClassName = 0;

    if (newClass->wc.style & CS_CLASSDC)
	newClass->hdce = DCE_AllocDCE( DCE_CLASS_DC );
    else newClass->hdce = 0;

      /* Make a copy of the menu name (only if it is a string) */

    if (HIWORD(class->lpszMenuName))
    {
//        char *menuname = PTR_SEG_TO_LIN( class->lpszMenuName );
	HANDLE hname = LocalAlloc(LMEM_FIXED, lstrlen(class->lpszMenuName)+1 );
	if (hname)
	{
	    newClass->wc.lpszMenuName = LocalLock(hname);//(SEGPTR)USER_HEAP_SEG_ADDR( hname );
	    lstrcpy( newClass->wc.lpszMenuName/*LocalLock( hname )*/, class->lpszMenuName/*menuname*/ );
	}
    }

    if (classExtra) _fmemset( newClass->wExtra, 0, classExtra );
    firstClass = handle;
	FUNCTION_END
    return newClass->atomName;
}


/***********************************************************************
 *           UnregisterClass    (USER.403)
 */
BOOL WINAPI UnregisterClass( LPCSTR className, HINSTANCE hinstance )
{
    HANDLE class, prevClass;
    CLASS * classPtr, * prevClassPtr;
    
    hinstance = GetExePtr( hinstance );
      /* Check if we can remove this class */
    class = CLASS_FindClassByName( className, hinstance, &classPtr );
    if (!class) return FALSE;
    if ((classPtr->wc.hInstance != hinstance) || (classPtr->cWindows > 0))
	return FALSE;
    
      /* Remove the class from the linked list */
    if (firstClass == class) firstClass = classPtr->hNext;
    else
    {
	for (prevClass = firstClass; prevClass; prevClass=prevClassPtr->hNext)
	{
	    prevClassPtr = (CLASS *) LocalLock(prevClass);
	    if (prevClassPtr->hNext == class) break;
	}
	if (!prevClass)
	{
	    TRACE("ERROR: Class list corrupted\n" );
	    return FALSE;
	}
	prevClassPtr->hNext = classPtr->hNext;
    }

      /* Delete the class */
    if (classPtr->hdce) DCE_FreeDCE( classPtr->hdce );
    if (classPtr->wc.hbrBackground) DeleteObject( classPtr->wc.hbrBackground );
    GlobalDeleteAtom( classPtr->atomName );
    if (HIWORD(classPtr->wc.lpszMenuName))
	LocalFree( (HANDLE)classPtr->wc.lpszMenuName );
    LocalFree( class );
    return TRUE;
}


/***********************************************************************
 *           GetClassWord    (USER.129)
 */
WORD WINAPI GetClassWord( HWND hwnd, int offset )
{
    return (WORD)GetClassLong( hwnd, offset );
}

#if 0
/***********************************************************************
 *           SetClassWord    (USER.130)
 */
WORD WINAPI SetClassWord( HWND hwnd, int offset, WORD newval )
{
    CLASS * classPtr;
    WND * wndPtr;
    WORD *ptr, retval = 0;
    
    if (!(wndPtr = WIN_FindWndPtr( hwnd ))) return 0;
    if (!(classPtr = CLASS_FindClassPtr( wndPtr->hClass ))) return 0;
    ptr = (WORD *)(((char *)classPtr->wExtra) + offset);
    retval = *ptr;
    *ptr = newval;
    return retval;
}


/***********************************************************************
 *           GetClassLong    (USER.131)
 */
LONG WINAPI GetClassLong( HWND hwnd, int offset )
{
    CLASS * classPtr;
    WND * wndPtr;
    
    if (!(wndPtr = WIN_FindWndPtr( hwnd ))) return 0;
    if (!(classPtr = CLASS_FindClassPtr( wndPtr->hClass ))) return 0;
    return *(LONG *)(((char *)classPtr->wExtra) + offset);
}


/***********************************************************************
 *           SetClassLong    (USER.132)
 */
LONG WINAPI SetClassLong( HWND hwnd, int offset, LONG newval )
{
    CLASS * classPtr;
    WND * wndPtr;
    LONG *ptr, retval = 0;
    
    if (!(wndPtr = WIN_FindWndPtr( hwnd ))) return 0;
    if (!(classPtr = CLASS_FindClassPtr( wndPtr->hClass ))) return 0;
    ptr = (LONG *)(((char *)classPtr->wExtra) + offset);
    retval = *ptr;
    *ptr = newval;
    return retval;
}


/***********************************************************************
 *           GetClassName      (USER.58)
 */
int GetClassName(HWND hwnd, LPSTR lpClassName, short maxCount)
{
    WND *wndPtr;
    CLASS *classPtr;

    /* FIXME: We have the find the correct hInstance */
    dprintf_class(stddeb,"GetClassName(%04x,%p,%d)\n",hwnd,lpClassName,maxCount);
    if (!(wndPtr = WIN_FindWndPtr(hwnd))) return 0;
    if (!(classPtr = CLASS_FindClassPtr(wndPtr->hClass))) return 0;
    
    return GlobalGetAtomName(classPtr->atomName, lpClassName, maxCount);
}

#endif

/***********************************************************************
 *           GetClassInfo      (USER.404)
 */
BOOL WINAPI GetClassInfo( HANDLE hInstance, LPCSTR name, LPWNDCLASS lpWndClass )
{
    CLASS *classPtr;

    TRACE("GetClassInfo: hInstance=%x className=%S",
		   hInstance,
                   HIWORD(name) ? name : "(int)" );

    hInstance = GetExePtr( hInstance );
    
    if (!(CLASS_FindClassByName( name, hInstance, &classPtr))) return FALSE;
    if (hInstance && (hInstance != classPtr->wc.hInstance)) return FALSE;

    _fmemcpy(lpWndClass, &(classPtr->wc), sizeof(WNDCLASS));
    return TRUE;
}

