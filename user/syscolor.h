/*
 * System color objects
 *
 * Copyright  Alexandre Julliard, 1994
 *
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

#ifndef SYSCOLOR_H
#define SYSCOLOR_H

#include <windows.h>
#include "display.h"

#if (WINVER >= 0x0300) && (WINVER < 0x030a)
#define NUM_SYS_COLORS     (COLOR_BTNTEXT+1)
#endif

#if WINVER >= 0x030a
#define NUM_SYS_COLORS     (COLOR_BTNHIGHLIGHT+1)
#endif

struct SysColorObjects
{
    HBRUSH hbrushScrollbar;        /* COLOR_SCROLLBAR           */
                                   /* COLOR_BACKGROUND          */
    HBRUSH hbrushActiveCaption;    /* COLOR_ACTIVECAPTION       */
    HBRUSH hbrushInactiveCaption;  /* COLOR_INACTIVECAPTION     */
    HBRUSH hbrushMenu;             /* COLOR_MENU                */
    HBRUSH hbrushWindow;           /* COLOR_WINDOW              */
    HPEN hpenWindowFrame;          /* COLOR_WINDOWFRAME         */
                                   /* COLOR_MENUTEXT            */
    HPEN hpenWindowText;           /* COLOR_WINDOWTEXT          */
                                   /* COLOR_CAPTIONTEXT         */
    HBRUSH hbrushActiveBorder;     /* COLOR_ACTIVEBORDER        */
    HBRUSH hbrushInactiveBorder;   /* COLOR_INACTIVEBORDER      */
                                   /* COLOR_APPWORKSPACE        */
    HBRUSH hbrushHighlight;        /* COLOR_HIGHLIGHT           */
                                   /* COLOR_HIGHLIGHTTEXT       */
    HBRUSH hbrushBtnFace;          /* COLOR_BTNFACE             */
    HBRUSH hbrushBtnShadow;        /* COLOR_BTNSHADOW           */
                                   /* COLOR_GRAYTEXT            */
                                   /* COLOR_BTNTEXT             */
                                   /* COLOR_INACTIVECAPTIONTEXT */
    HBRUSH hbrushBtnHighlight;     /* COLOR_BTNHIGHLIGHT        */
};

extern VOID FAR SYSCOLOR_Init(VOID);
extern struct SysColorObjects sysColorObjects;

#endif  /* SYSCOLOR_H */
