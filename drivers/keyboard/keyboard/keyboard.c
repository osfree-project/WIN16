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

#include <dos.h>

#include <windows.h>

#include "vkeys.h"

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

static FARPROC DefKeybEventProc=NULL;
static LPBYTE pKeyStateTable=NULL;
static WORD wScreenSwitchEnable=1;	// Screen switch enabled by default
static void far * lpOldInt09=NULL;		// Old INT 09H handler
static BYTE fSysReq=0;			// Enables CTRL-ALT-SysReq if NZ
static UINT KeyboardType=4;		// Keyboard type, detected by driver or from system.ini
static UINT KeyboardSubType=0;		// Keyboard sub type, detected by driver or from system.ini

extern BYTE near * PASCAL tablestart;	// OEM<>ANSI XLAT tables

// This table used by keyboard interrupt handler to produce virtual key
// code VK_? from keyboard scan code. This table can be changed by NewTable
// function. Also MapVirtualKey uses it.

static BYTE keyTrTab[89] ={
		-1			,
		VK_ESCAPE 	,
		VK_1      	,// 02h 2
		VK_2      	,// 03h 3
		VK_3      	,// 04h 4
		VK_4      	,// 05h 5
		VK_5      	,// 06h 6
		VK_6      	,// 07h 7
		VK_7      	,// 08h 8
		VK_8      	,// 09h 9
		VK_9      	,// 0ah 10
		VK_0      	,// 0bh 11
		VK_OEM_MINUS	,// 0ch 12	; variable
		VK_OEM_PLUS 	,// 0dh 13	; variable
		VK_BACK   	,// 0eh 14
		VK_TAB    	,// 0fh 15
		VK_Q      	,// 010h 16	; variable -- also VK_A
		VK_W      	,// 011h 17	; variable -- also VK_Z
		VK_E      	,// 012h 18
		VK_R      	,// 013h 19
		VK_T      	,// 014h 20
		VK_Y      	,// 015h 21	; variable -- also VK_Z
		VK_U      	,// 016h 22
		VK_I      	,// 017h 23
		VK_O      	,// 018h 24
		VK_P      	,// 019h 25
		VK_OEM_4    	,// 01ah 26	; variable
		VK_OEM_6    	,// 01bh 27	; variable
		VK_RETURN 	,// 01ch 28
		VK_CONTROL	,// 01dh 29
		VK_A      	,// 01eh 30	; variable -- also VK_Q
		VK_S      	,// 01fh 31
		VK_D      	,// 020h 32
		VK_F      	,// 021h 33
		VK_G      	,// 022h 34
		VK_H      	,// 023h 35
		VK_J      	,// 024h 36
		VK_K      	,// 025h 37
		VK_L      	,// 026h 38
		VK_OEM_1    	,// 027h 39	; variable -- also VK_M
		VK_OEM_7    	,// 028h 40	; variable
//; X1	label byte	; swap for AT (no swap for USA version)
		VK_OEM_3    	,// 029h 41	; variable
		VK_SHIFT  	,// 02ah 42
// X2	label byte	; swap for AT
		VK_OEM_5    	,// 02bh 43	; variable
		VK_Z      	,// 02ch 44	; variable -- also VK_Y
		VK_X      	,// 02dh 45
		VK_C      	,// 02eh 46
		VK_V      	,// 02fh 47
		VK_B      	,// 030h 48
		VK_N      	,// 031h 49
		VK_M      	,// 032h 50	; variable --
		VK_OEM_COMMA	,// 033h 51	; variable
		VK_OEM_PERIOD	,// 034h 52	; variable
		VK_OEM_2    	,// 035h 53	; variable
		VK_SHIFT  	,// 036h 54
		VK_MULTIPLY	,// 037h 55
		VK_MENU   	,// 038h 56
		VK_SPACE  	,// 039h 57
		VK_CAPITAL	,// 03ah 58
		VK_F1     	,// 03bh 59
		VK_F2     	,// 03ch 60
		VK_F3     	,// 03dh 61
		VK_F4     	,// 03eh 62
		VK_F5     	,// 03fh 63
		VK_F6     	,// 040h 64
		VK_F7     	,// 041h 65
		VK_F8     	,// 042h 66
		VK_F9     	,// 043h 67
		VK_F10    	,// 044h 68
	//; (scancodes 69..83 for most keyboards -- overlaid for Nokia type 6)
//LabNok6 label byte
		VK_NUMLOCK	,// 045h 69
		VK_OEM_SCROLL	,// 046h 70
		VK_HOME   	,// 047h 71
		VK_UP     	,// 048h 72
		VK_PRIOR  	,// 049h 73
		VK_SUBTRACT	,// 04ah 74
		VK_LEFT   	,// 04bh 75
		VK_CLEAR  	,// 04ch 76
		VK_RIGHT  	,// 04dh 77
		VK_ADD    	,// 04eh 78
		VK_END    	,// 04fh 79
		VK_DOWN   	,// 050h 80
		VK_NEXT   	,// 051h 81
		VK_INSERT 	,// 052h 82
		VK_DELETE 	,// 053h 83
// Enhanced keyboard
		-1		,// 054h 84
		-1		,// 055h 85
		VK_OEM_102	,// 056h 86
		VK_F11    	,// 057h 87
		VK_F12    	// 058h 88
};

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

void far * mymemset (void far *start, int c, int len)
{
  char far *p = start;

  while (len -- > 0)
    *p ++ = c;

  return start;
}

/***********************************************************************
 *		Enable (KEYBOARD.2)
 */
VOID WINAPI Enable( FARPROC proc, LPBYTE lpKeyState )
{
    DefKeybEventProc = proc;
    pKeyStateTable = lpKeyState;

    mymemset( lpKeyState, 0, 256 ); /* all states to false */
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
// @todo Must be int for return
void WINAPI AnsiToOem( const char huge * s, char huge *  d )
{
    char huge * dst;

      dst = (char huge *)s;
      while( *dst )
      {
        if ((BYTE)*dst<=0x1f)		// Range 0x00-0x1f
        {
           *d=tablestart[*dst+2];
        } else 
        if ((BYTE)*dst>=0x80)		// Range 0x80-0xff
        {
           *d=tablestart[*dst+2+0x20-0x80];
        } else {			// Range 0x20-0x7f
          *d=*dst;
        }
        d++;
        dst++;
      }

// @todo always return -1
//    return -1;
}

/***********************************************************************
 *           OemToAnsi   (KEYBOARD.6)
 */
// @todo Must be int for return
void WINAPI OemToAnsi( const char huge *  s, char huge *  d )
{
    char huge * dst;
      dst = (char huge *)s;
      while( *dst )
      {
        if ((BYTE)*dst<=0x1f)		// Range 0x00-0x1f
        {
           *d=tablestart[*dst+2+0x20+128];
        } else 
        if ((BYTE)*dst>=0x80)		// Range 0x80-0xff
        {
           *d=tablestart[*dst+2+0x20+128+0x20-0x80];
        } else {			// Range 0x20-0x7f
          *d=*dst;
        }
        d++;
        dst++;
      }

// @todo always return -1
//    return -1;
}

/**********************************************************************
 *		SetSpeed (KEYBOARD.7)
 */
WORD WINAPI SetSpeed(WORD unused)
{
//    FIXME("(%04x): stub\n", unused);
/*
INT 16,3 - Set Keyboard Typematic Rate (AT+)

	AH = 03
	AL = 00  set typematic rate to default
	     01  increase initial delay
	     02  slow typematic rate by 1/2
	     04  turn off typematic chars
	     05  set typematic rate/delay

	BH = repeat delay (AL=5)
	     0 = 250ms	   2 = 750ms
	     1 = 500ms	   3 = 1000ms
	BL = typematic rate, one of the following  (AL=5)

	     00 - 30.0	    01 - 26.7	   02 - 24.0	  03 - 21.8
	     04 - 20.0	    05 - 18.5	   06 - 17.1	  07 - 16.0
	     08 - 15.0	    09 - 13.3	   0A - 12.0	  0B - 10.9
	     0C - 10.0	    0D - 9.2	   0E - 8.6	  0F - 8.0
	     10 - 7.5	    11 - 6.7	   12 - 6.0	  13 - 5.5
	     14 - 5.0	    15 - 4.6	   16 - 4.3	  17 - 4.0
	     18 - 3.7	    19 - 3.3	   1A - 3.0	  1B - 2.7
*/
    return 0xffff;
}

/**********************************************************************
 *		ScreenSwitchEnable (KEYBOARD.100)
 *
 *   The fEnable parameter is set to 0 to disable
 *   screen switches, and a NONZERO value to re-enable them.
 *   At startup, screen switches are enabled.
 *
 *   This flag is for OS/2 1.x DOS Compatibility Box.
 *
 */
VOID WINAPI ScreenSwitchEnable(WORD fEnable)
{
	wScreenSwitchEnable=fEnable;
}

/***********************************************************************
 *           GetTableSeg   (KEYBOARD.126)
 *
 *  This function used to initialize internal table segment variable to
 *  internal tables. As side result returns it on exit.
 *
 */
WORD WINAPI GetTableSeg(VOID)
{
	// @todo Not correct yet
	return SELECTOROF(tablestart);
}

/***************************************************************************
 *		NewTable (KEYBOARD.127)
 *
 *	Change keyboard tables, if a keyboard table DLL is defined in
 *	SYSTEM.INI and the function GetKbdTable() exists and returns
 *	successfully.
 *
 *	This function is passed no parameters by the caller -- it obtains
 *	the following from SYSTEM.INI:
 *
 *	      [keyboard]
 *		TYPE = 4			; 1..6.  4 is enhanced kbd.
 *		SUBTYPE = 0			; 0 for all but Olivetti
 *						; 8086 systems & AT&T 6300+
 *		KEYBOARD.DLL = kbdus.dll	; name of DLL file
 *		OEMANSI.BIN = XLATNO.BIN	; oem/ansi tables file
 *
 *	The module name of the DLL is expected to be the root of the DLL's
 *	file name.  In any event, the module name must be different for
 *	each keyboard-table DLL!
 *
 */
VOID WINAPI NewTable(VOID)
{
    if (!KeyboardType)
    {
      KeyboardType=GetPrivateProfileInt("keyboard", "type", 4, "SYSTEM.INI");
      KeyboardSubType=GetPrivateProfileInt("keyboard", "subtype", 0, "SYSTEM.INI");
    }
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
// @todo This is not correct way to detect keyboard. Normal keyboard detection
// must be in NewTable function.
int WINAPI GetKeyboardType(int nTypeFlag)
{
    switch(nTypeFlag)
    {
    case 0:      /* Keyboard type */
        return KeyboardType;
    case 1:      /* Keyboard Subtype */
        return KeyboardSubType;
    case 2:      /* Number of F-keys */
        return 12;   /* We're doing an 101 for now, so return 12 F-keys */
    }

    return 0;    /* The book says 0 here, so 0 */
}

/******************************************************************************
 *		MapVirtualKey (KEYBOARD.131)
 *
 * MapVirtualKey translates keycodes from one format to another
 * MAPVK_VK_TO_VSC 0 The uCode parameter is a virtual-key code and is 
 *                   translated into a scan code. If it is a virtual-key 
 *                   code that does not distinguish between left- and 
 *                   right-hand keys, the left-hand scan code is returned.
 *                   If there is no translation, the function returns 0.
 * MAPVK_VSC_TO_VK 1 The uCode parameter is a scan code and is translated
 *                   into a virtual-key code that does not distinguish 
 *                   between left- and right-hand keys. If there is no 
 *                   translation, the function returns 0.
 * MAPVK_VK_TO_CHAR 2 The uCode parameter is a virtual-key code and is 
 *                   translated into an unshifted character value in the
 *                   low order word of the return value. Dead keys 
 *                   (diacritics) are indicated by setting the top 
 *                   bit of the return value. If there is no translation, 
 *                   the function returns 0. See Remarks.
 *
 * @todo only wMapType=1 supported for now
 */
UINT WINAPI MapVirtualKey(UINT wCode, UINT wMapType)
{
	if (wMapType==2) return keyTrTab[wCode];
	return 0;
}

/****************************************************************************
 *		GetKBCodePage (KEYBOARD.132)
 */
int WINAPI GetKBCodePage(void)
{
	return (WORD)*tablestart;
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
    char huge * dst;

      dst = (char huge *)s;
      while( len )
      {
        if ((BYTE)*dst<=0x1f)		// Range 0x00-0x1f
        {
           *d=tablestart[*dst+2];
        } else 
        if ((BYTE)*dst>=0x80)		// Range 0x80-0xff
        {
           *d=tablestart[*dst+2+0x20-0x80];
        } else {			// Range 0x20-0x7f
          *d=*dst;
        }
        len--;
	d++;
        dst++;
      }
}

/***********************************************************************
 *           OemToAnsiBuff   (KEYBOARD.135)
 */
void WINAPI OemToAnsiBuff( LPCSTR s, LPSTR d, UINT len )
{
    char huge * dst;
      dst = (char huge *)s;
      while( len )
      {
        if ((BYTE)*dst<=0x1f)		// Range 0x00-0x1f
        {
           *d=tablestart[*dst+2+0x20+128];
        } else 
        if ((BYTE)*dst>=0x80)		// Range 0x80-0xff
        {
           *d=tablestart[*dst+2+0x20+128+0x20-0x80];
        } else {			// Range 0x20-0x7f
          *d=*dst;
        }
        len--;
        d++;
        dst++;
      }
}

/***********************************************************************
 *           EnableKBSysReq   (KEYBOARD.136)
 *
 *   This function enables and shuttles off NMI interrupt simulation
 *   (trap to debugger) when CTRL-ALT-SysReq is pressed.
 *   CVWBreak overides int 2.
 *   fSys	= 01	enable	int 2
 *		= 02	disable int 2
 *		= 04	enable	CVWBreak
 *		= 08	disable CVWBreak
 *
 */

BYTE WINAPI EnableKBSysReq(WORD fSys)
{
	switch(fSys)
	{
		case 1: fSysReq=fSysReq | 1;
		        break;
		case 2: fSysReq=fSysReq & !1;
		        break;
		case 4: fSysReq=fSysReq | 2;
		        break;
		case 8: fSysReq=fSysReq & !2;
		        break;
	}
	return fSysReq;
}

/***********************************************************************
 *           GetBIOSKeyProc   (KEYBOARD.137)
 */
FARPROC WINAPI GetBIOSKeyProc(VOID)
{
	return lpOldInt09;
}

void interrupt far interrupt_09_handler(void)
{
    /* do something */
    _asm {
      call lpOldInt09
    }
}

extern  void (interrupt far *_dos_getvect( int ax ))();
#pragma aux  _dos_getvect = \
    "mov ah,35h"        \
    "int 21h"           \
    parm [ax] value [es bx];

extern  void _dos_setvect( int, void (interrupt far *)());
#pragma aux  _dos_setvect = \
    "push ds"           \
    "mov ds,cx"         \
    "mov ah,25h"        \
    "int 21h"           \
    "pop ds"            \
    parm caller [ax] [cx dx];

void KbdInit(void)
{
    lpOldInt09 = _dos_getvect(9);
    _dos_setvect(9, interrupt_09_handler);
}

BOOL WINAPI LibMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved)
{
	return TRUE;
}
