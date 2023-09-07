/*
 * PROJECT:     ReactOS Clipboard Viewer
 * LICENSE:     GPL-2.0+ (https://spdx.org/licenses/GPL-2.0+)
 * PURPOSE:     Clipboard helper functions.
 * COPYRIGHT:   Copyright 2015-2018 Ricardo Hanke
 *              Copyright 2015-2018 Hermes Belusca-Maito
 */

#pragma once

LRESULT
SendClipboardOwnerMessage(
    BOOL bUnicode,
    UINT uMsg,
    WPARAM wParam,
    LPARAM lParam);

void
RetrieveClipboardFormatName(HINSTANCE hInstance,
                            UINT uFormat,
                            BOOL Unicode,
                            LPSTR lpszFormat,
                            UINT cch);

void DeleteClipboardContent(void);
UINT GetAutomaticClipboardFormat(void);
BOOL IsClipboardFormatSupported(UINT uFormat);

size_t
GetLineExtentW(
    LPCSTR lpText,
    LPCSTR* lpNextLine);

size_t
GetLineExtentA(
    LPCSTR lpText,
    LPCSTR* lpNextLine);

BOOL GetClipboardDataDimensions(UINT uFormat, PRECT pRc);
