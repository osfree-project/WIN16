/*
 * PROJECT:     ReactOS Clipboard Viewer
 * LICENSE:     GPL-2.0+ (https://spdx.org/licenses/GPL-2.0+)
 * PURPOSE:     Precompiled header.
 * COPYRIGHT:   Copyright 2015-2018 Ricardo Hanke
 */

#ifndef _CLIPBRD_PCH_
#define _CLIPBRD_PCH_

// #pragma once

#include <limits.h>
#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#define MAX_PATH FILENAME_MAX
#include <string.h>

#include <windows.h>
#include <shellapi.h>
#include <commdlg.h>

#include "resources.h"
#include "cliputils.h"
#include "fileutils.h"
#include "scrollutils.h"
#include "winutils.h"
#define MAX_STRING_LEN      255
#define DISPLAY_MENU_POS 2

#define CF_NONE 0

#define ARRAYSIZE(a) sizeof(a)/sizeof((a)[0])

typedef struct _CLIPBOARD_GLOBALS
{
    HINSTANCE hInstance;
    HWND hMainWnd;
    HWND hWndNext;
    HMENU hMenu;
    UINT uDisplayFormat;
    UINT uCheckedItem;

    /* Metrics of the current font */
    LONG CharWidth;
    LONG CharHeight;
} CLIPBOARD_GLOBALS;

extern CLIPBOARD_GLOBALS Globals;

int WINAPI ShellAbout(HWND hWnd, LPCSTR lpszCaption, LPCSTR lpszAboutText,
                HICON hIcon);

#endif /* _CLIPBRD_PCH_ */
