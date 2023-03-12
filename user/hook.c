/*
 * Windows 16-bit hook functions
 *
 * Copyright 1994, 1995, 2002 Alexandre Julliard
 * Copyright 1996 Andrew Lewycky
 *
 * Based on investigations by Alex Korobka
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301, USA
 */

#include <stdarg.h>

#include <windows.h>

HANDLE WINAPI FarGetOwner( HGLOBAL handle );

/***********************************************************************
 *		SetWindowsHook (USER.121)
 */
FARPROC WINAPI SetWindowsHook( int id, HOOKPROC proc )
{
    HINSTANCE hInst = FarGetOwner( HIWORD(proc) );

    /* WH_MSGFILTER is the only task-specific hook for SetWindowsHook() */
    HTASK hTask = (id == WH_MSGFILTER) ? GetCurrentTask() : 0;

    return (FARPROC)SetWindowsHookEx( id, proc, hInst, hTask );
}



/***********************************************************************
 *		DefHookProc (USER.235)
 */
LRESULT WINAPI DefHookProc( int code, WPARAM wparam, LPARAM lparam, HOOKPROC far *hhook )
{
    return CallNextHookEx( (HHOOK)*hhook, code, wparam, lparam );
}
