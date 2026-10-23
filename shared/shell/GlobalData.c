/*
 * 				Shell Library Functions
 *
 * Copyright 1993, 1994, 1995 Alexandre Julliard
 * Copyright 1997 Willows Software, Inc. 
 * Copyright 1998 Marcus Meissner
 * Copyright 2000 Juergen Schmied
 * Copyright 2002 Eric Pouech
 * Copyright 2023 Yuri Prokushev
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
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

/*! @file GlobalData.c
    @brief Global state shared by the shell library modules.
*/

#include "Shell.h"

/*! @brief Global state (instance handle, registry handles). */
GLOBALS Globals;

/*! @brief win.ini [windows] section name. */
char szWindows[20];
/*! @brief win.ini [windows] Programs key name. */
char szPrograms[20];
/*! @brief Default value of win.ini [windows] Programs key. */
char szProgramsValue[20];
/*! @brief win.ini [extensions] section name. */
char szExtensions[20];
/*! @brief Verb name for opening a file ("open"). */
char szOpen[20];

/*! @brief Shell window registered for system-wide notifications. */
HWND SHELL_hWnd=0;
/*! @brief Handle of the installed WH_SHELL hook. */
HHOOK SHELL_hHook=0;
/*! @brief Thunked hook procedure for the installed WH_SHELL hook. */
HOOKPROC SHELL_lpProc=NULL;

/*! @brief Registered message: OTHERWINDOWCREATED. */
UINT	uMsgWndCreated = 0;
/*! @brief Registered message: OTHERWINDOWDESTROYED. */
UINT	uMsgWndDestroyed = 0;
/*! @brief Registered message: ACTIVATESHELLWINDOW. */
UINT	uMsgShellActivate = 0;

/*! @brief Cached display device context. */
HDC display_dc;

/*! @brief Global atom table used by AddAtomEx/FindAtomEx. */
ATOMTABLE AtomTable;

/*! @brief TRUE once the registry has been initialised. */
BOOL fRegInitialized = FALSE;

/*! @brief Root key of the in-memory registry. */
KEYSTRUCT RootKey;

/*! @brief Text of the OTHERWINDOWCREATED system message. */
const char lpstrMsgWndCreated[] = "OTHERWINDOWCREATED";
/*! @brief Text of the OTHERWINDOWDESTROYED system message. */
const char lpstrMsgWndDestroyed[] = "OTHERWINDOWDESTROYED";
/*! @brief Text of the ACTIVATESHELLWINDOW system message. */
const char lpstrMsgShellActivate[] = "ACTIVATESHELLWINDOW";
