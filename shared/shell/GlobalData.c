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

#include "Shell.h"

GLOBALS Globals;

char szWindows[20];
char szPrograms[20];
char szProgramsValue[20];
char szExtensions[20];
char szOpen[20];

HWND SHELL_hWnd=0;
HHOOK SHELL_hHook=0;
HOOKPROC SHELL_lpProc=NULL;

UINT	uMsgWndCreated = 0;
UINT	uMsgWndDestroyed = 0;
UINT	uMsgShellActivate = 0;

HDC display_dc;

ATOMTABLE AtomTable;

BOOL fRegInitialized = FALSE;

KEYSTRUCT RootKey;

const char lpstrMsgWndCreated[] = "OTHERWINDOWCREATED";
const char lpstrMsgWndDestroyed[] = "OTHERWINDOWDESTROYED";
const char lpstrMsgShellActivate[] = "ACTIVATESHELLWINDOW";
