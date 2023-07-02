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
#include <windows.h>

#include <win_private.h>

/***********************************************************************
 *           GetProfileInt   (KERNEL.57)
 */
UINT WINAPI GetProfileInt( LPCSTR section, LPCSTR entry, int def_val )
{
    UINT res;
    FUNCTIONSTART;

    res=GetPrivateProfileInt( section, entry, def_val, "win.ini" );

    FUNCTIONEND;
    return res;
}

/***********************************************************************
 *           GetPrivateProfileInt   (KERNEL.127)
 */
UINT WINAPI GetPrivateProfileInt( LPCSTR section, LPCSTR entry,
                                      int def_val, LPCSTR filename )
{
    char buffer[30];

    FUNCTIONSTART;

    /* we used to have some elaborate return value limitation (<= -32768 etc.)
     * here, but Win98SE doesn't care about this at all, so I deleted it.
     * AFAIR versions prior to Win9x had these limits, though. */


    if (GetPrivateProfileString( section, entry, "", buffer, sizeof( buffer ), filename ) == 0)
    {
        FUNCTIONEND;
        return def_val;
    }

    /* FIXME: if entry can be found but it's empty, then Win16 is
     * supposed to return 0 instead of def_val ! Difficult/problematic
     * to implement (every other failure also returns zero buffer),
     * thus wait until testing framework avail for making sure nothing
     * else gets broken that way. */
    if (!buffer[0]) 
    {
        FUNCTIONEND;
        return (UINT)def_val;
    }

    FUNCTIONEND;
    return atoi(buffer);
}

/***********************************************************************
 *           GetProfileString   (KERNEL.58)
 */
int WINAPI GetProfileString( LPCSTR section, LPCSTR entry, LPCSTR def_val,
                                 LPSTR buffer, int len )
{
    int res;

    FUNCTIONSTART;

    res=GetPrivateProfileString( section, entry, def_val,
                                      buffer, len, "win.ini");

    FUNCTIONEND;
    return res;
}


/***********************************************************************
 *           WriteProfileString   (KERNEL.59)
 */
BOOL WINAPI WriteProfileString( LPCSTR section, LPCSTR entry,
                                    LPCSTR string )
{
    BOOL res;
    FUNCTIONSTART;

    res=WritePrivateProfileString( section, entry, string, "win.ini" );

    FUNCTIONEND;
    return res;
}
