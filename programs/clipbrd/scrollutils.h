/*
 * PROJECT:     ReactOS Clipboard Viewer
 * LICENSE:     GPL-2.0+ (https://spdx.org/licenses/GPL-2.0+)
 * PURPOSE:     Scrolling related helper functions.
 * COPYRIGHT:   Copyright 2015-2018 Ricardo Hanke
 *              Copyright 2015-2018 Hermes Belusca-Maito
 */

#pragma once

typedef struct _SCROLLSTATE
{
    UINT uLinesToScroll;    /* Number of lines to scroll on one wheel rotation movement (== one "click" == WHEEL_DELTA ticks) */
    int iWheelCarryoverX;   /* Unused wheel ticks (< WHEEL_DELTA) */
    int iWheelCarryoverY;
    int nPageX;             /* Number of lines per page */
    int nPageY;
    int CurrentX;           /* Current scrollbar position */
    int CurrentY;
    int MaxX;               /* Maximum scrollbar position */
    int MaxY;
    int nMaxWidth;          /* Maximum span of displayed data */
    int nMaxHeight;
} SCROLLSTATE, *LPSCROLLSTATE;

void OnKeyScroll(HWND hWnd, WPARAM wParam, LPARAM lParam, LPSCROLLSTATE state);
void OnMouseScroll(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, LPSCROLLSTATE state);
void OnScroll(HWND hWnd, int nBar, WPARAM wParam, int iDelta, LPSCROLLSTATE state);

void UpdateLinesToScroll(LPSCROLLSTATE state);
void UpdateWindowScrollState(HWND hWnd, int nMaxWidth, int nMaxHeight, LPSCROLLSTATE lpState);
