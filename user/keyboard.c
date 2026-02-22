/*
 * Keyboard related functions
 *
 * Copyright 1993 Bob Amstadt
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
 * License along with this library; if not, see
 * <https://www.gnu.org/licenses/>.
 *
 */
#include "user.h"

/**********************************************************************
 *		GetKeyState			[USER.106]
 */
int WINAPI GetKeyState(int keycode)
{
    int retval;

    switch(keycode) {
     case VK_LBUTTON:
	return MouseButtonsStates[0];
     case VK_MBUTTON:
	return MouseButtonsStates[1];
     case VK_RBUTTON:
	return MouseButtonsStates[2];
     default:
	retval = ( (int)(KeyStateTable[keycode] & 0x80) << 8 ) |
		   (int)(KeyStateTable[keycode] & 0x01);
    }

    return retval;
}

/**********************************************************************
 *		GetKeyboardState			[USER.222]
 */
void WINAPI GetKeyboardState(BYTE FAR *lpKeyState)
{
    if (lpKeyState != NULL) {
	KeyStateTable[VK_LBUTTON] = MouseButtonsStates[0];
	KeyStateTable[VK_MBUTTON] = MouseButtonsStates[1];
	KeyStateTable[VK_RBUTTON] = MouseButtonsStates[2];
	_fmemcpy(lpKeyState, KeyStateTable, 256);
    }
}

/**********************************************************************
 *      SetKeyboardState            [USER.223]
 */
void WINAPI SetKeyboardState(BYTE FAR *lpKeyState)
{
    if (lpKeyState != NULL) {
	_fmemcpy(KeyStateTable, lpKeyState, 256);
	MouseButtonsStates[0] = KeyStateTable[VK_LBUTTON];
	MouseButtonsStates[1] = KeyStateTable[VK_MBUTTON];
	MouseButtonsStates[2] = KeyStateTable[VK_RBUTTON];
    }
}

/**********************************************************************
 *            GetAsyncKeyState        (USER.249)
 *
 *	Determine if a key is or was pressed.  retval has high-order 
 * byte set to 1 if currently pressed, low-order byte 1 if key has
 * been pressed.
 *
 *	This uses the variable AsyncMouseButtonsStates and
 * AsyncKeyStateTable (set in event.c) which have the mouse button
 * number or key number (whichever is applicable) set to true if the
 * mouse or key had been depressed since the last call to 
 * GetAsyncKeyState.
 */
int WINAPI GetAsyncKeyState(int nKey)
{
    short retval;	

    switch (nKey) {
     case VK_LBUTTON:
	retval = AsyncMouseButtonsStates[0] | 
	(MouseButtonsStates[0] << 8);
	break;
     case VK_MBUTTON:
	retval = AsyncMouseButtonsStates[1] |
	(MouseButtonsStates[1] << 8);
	break;
     case VK_RBUTTON:
	retval = AsyncMouseButtonsStates[2] |
	(MouseButtonsStates[2] << 8);
	break;
     default:
	retval = AsyncKeyStateTable[nKey] | 
	(KeyStateTable[nKey] << 8);
	break;
    }

    _fmemset( AsyncMouseButtonsStates, 0, 3 );  /* all states to false */
    _fmemset( AsyncKeyStateTable, 0, 256 );

    return retval;
}

