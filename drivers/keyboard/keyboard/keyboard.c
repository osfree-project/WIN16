/*
 * KEYBOARD driver
 *
 * Copyright 1993 Bob Amstadt
 * Copyright 1996 Albrecht Kleine
 * Copyright 1997 David Faure
 * Copyright 1998 Morten Welinder
 * Copyright 1998 Ulrich Weigand
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
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "windows.h"

//WINE_DEFAULT_DEBUG_CHANNEL(keyboard);

#pragma pack(push, 1)
typedef struct _KBINFO
{
    BYTE Begin_First_Range;
    BYTE End_First_Range;
    BYTE Begin_Second_Range;
    BYTE End_Second_Range;
    WORD StateSize;
} KBINFO, far *LPKBINFO;
#pragma pack(pop)

static FARPROC DefKeybEventProc;
static LPBYTE pKeyStateTable;

/***********************************************************************
 *		Inquire (KEYBOARD.1)
 */
WORD WINAPI Inquire(LPKBINFO kbInfo)
{
  kbInfo->Begin_First_Range = 0;
  kbInfo->End_First_Range = 0;
  kbInfo->Begin_Second_Range = 0;
  kbInfo->End_Second_Range = 0;
  kbInfo->StateSize = 16;

  return sizeof(KBINFO);
}

/***********************************************************************
 *		Enable (KEYBOARD.2)
 */
VOID WINAPI Enable( FARPROC proc, LPBYTE lpKeyState )
{
    DefKeybEventProc = proc;
    pKeyStateTable = lpKeyState;

    _fmemset( lpKeyState, 0, 256 ); /* all states to false */
}

/***********************************************************************
 *		Disable (KEYBOARD.3)
 */
VOID WINAPI Disable(VOID)
{
    DefKeybEventProc = NULL;
    pKeyStateTable = NULL;
}

/****************************************************************************
 *		ToAscii (KEYBOARD.4)
 */
int WINAPI ToAscii(UINT virtKey,UINT scanCode, LPBYTE lpKeyState,
                       unsigned long far * lpChar, UINT flags)
{
//    return ToAscii( virtKey, scanCode, lpKeyState, lpChar, flags );
}

/***********************************************************************
 *           AnsiToOem   (KEYBOARD.5)
 */
void WINAPI AnsiToOem( const char huge * s, char huge *  d )
{
        if(s != d)
                lstrcpy(d,s);
//    CharToOemA( s, d );
    //return -1;
}

/***********************************************************************
 *           OemToAnsi   (KEYBOARD.6)
 */
void WINAPI OemToAnsi( const char huge *  s, char huge *  d )
{
    if (s != d)
        lstrcpy(d,s);
//    OemToCharA( s, d );
    //return -1;
}

/**********************************************************************
 *		SetSpeed (KEYBOARD.7)
 */
WORD WINAPI SetSpeed(WORD unused)
{
//    FIXME("(%04x): stub\n", unused);
    return 0xffff;
}

/**********************************************************************
 *		ScreenSwitchEnable (KEYBOARD.100)
 */
VOID WINAPI ScreenSwitchEnable(WORD unused)
{
//  FIXME("(%04x): stub\n", unused);
}

/**********************************************************************
 *		OemKeyScan (KEYBOARD.128)
 */
DWORD WINAPI OemKeyScan(UINT wOemChar)
{
//    return OemKeyScan( wOemChar );
}

/**********************************************************************
 *		VkKeyScan (KEYBOARD.129)
 */
/*
 *      from keyboard driver...
 *      add the following:
 *              input   output
 *              0       -1
 *              1       'A' + CONTROL
 *              2       'B' + CONTROL
 *              3       3
 *              4-7     'D' + CONTROL, 'E', 'F', 'G'
 *              8       8
 *              9       9
 *              a       0xd + CONTROL
 *              b,c     'K' + CONTROL, 'L'
 *              d       0xd
 *              e-1a    'N' + CONTROL, 'LETTER' + CONTROL
 *              1e,1f   -1
 *              20-3f   VK_KEY w/wo shifts 'A'...
 *              40-5a   letter + SHIFT
 *              5b-60   VKKEY w/wo shifts
 *              60-7a   'A'-'Z' w/o shifts
 *              7b      0xdb
 *              7c      0xdd
 *              7d      0xc0
 *              7e      0x08    + CONTROL
 *              7f      0x08    + CONTROL
 */
UINT WINAPI VkKeyScan(UINT cChar)
{
        UINT   rc;

        rc = cChar & 0xff;

        /* A-Z VK is ascii code + shifted bit in upper byte */
        /* +0x100       vk is ascii code + shift key */
        if(rc >= 'A' && rc <= 'Z')
                return rc + 0x100;

        /* a-z VK is ascii code - 32 */
        /* +0x000       vk is ascii code */
        if(rc >= 'a' && rc <= 'z')
                return rc - 32;

        /* we need to calculate the remaining codes */
        /* +0x200       vk is ascii code + control key */
        /* +0x600       vk is ascii code + control&alt key */
        /* +0x700       vk is ascii code + shift&control&alt key */
        /* 0x300,0x400,0x500 are not used... */
        return -1;
//    return VkKeyScanA( cChar );
}

/******************************************************************************
 *		GetKeyboardType (KEYBOARD.130)
 */
int WINAPI GetKeyboardType(int nTypeFlag)
{
//    return GetKeyboardType( nTypeFlag );
}

/******************************************************************************
 *		MapVirtualKey (KEYBOARD.131)
 *
 * MapVirtualKey translates keycodes from one format to another
 */
UINT WINAPI MapVirtualKey(UINT wCode, UINT wMapType)
{
//    return MapVirtualKeyA(wCode,wMapType);
}

/****************************************************************************
 *		GetKBCodePage (KEYBOARD.132)
 */
int WINAPI GetKBCodePage(void)
{
//    return GetKBCodePage();
}

/****************************************************************************
 *		GetKeyNameText (KEYBOARD.133)
 */
int WINAPI GetKeyNameText(LONG lParam, LPSTR lpBuffer, int nSize)
{
//    return GetKeyNameTextA( lParam, lpBuffer, nSize );
}

/***********************************************************************
 *           AnsiToOemBuff   (KEYBOARD.134)
 */
void WINAPI AnsiToOemBuff( LPCSTR s, LPSTR d, UINT len )
{
    if (len != 0) //CharToOemBuffA( s, d, len );
        if(s != d)
                lstrcpyn(d,s,len);
}

/***********************************************************************
 *           OemToAnsiBuff   (KEYBOARD.135)
 */
void WINAPI OemToAnsiBuff( LPCSTR s, LPSTR d, UINT len )
{
    if (len != 0) //OemToCharBuffA( s, d, len );
        if(s != d)
                lstrcpyn(d,s,len);
}

BOOL WINAPI LibMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved)
{

    return TRUE;
}
