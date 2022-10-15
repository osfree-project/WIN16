/*
 * File handling functions
 *
 * Copyright 1993 John Burton
 * Copyright 1996 Alexandre Julliard
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
 *
 */

#include <i86.h>
#include <win16.h>

#if 0
/***********************************************************************
 *           GetProfileInt   (KERNEL.57)
 */
UINT WINAPI GetProfileInt( LPCSTR section, LPCSTR entry, int def_val )
{
    return GetPrivateProfileInt( section, entry, def_val, "win.ini" );
}
#endif

/***********************************************************************
 *           GetProfileString   (KERNEL.58)
 */
int WINAPI GetProfileString( LPCSTR section, LPCSTR entry, LPCSTR def_val,
                                 LPSTR buffer, int len )
{
    return GetPrivateProfileString( section, entry, def_val,
                                      buffer, len, "win.ini" );
}


/***********************************************************************
 *           WriteProfileString   (KERNEL.59)
 */
BOOL WINAPI WriteProfileString( LPCSTR section, LPCSTR entry,
                                    LPCSTR string )
{
    return WritePrivateProfileString( section, entry, string, "win.ini" );
}
