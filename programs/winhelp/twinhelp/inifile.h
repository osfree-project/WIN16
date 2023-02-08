/*************************************************************************
*
* INIFILE.H
* TWINHELP.INI file reading and writing routines.     
*
* %W% %G% %U% %P%
*
* Copyright (c) 1995-1996, Willows Software Inc.  All rights reserved.
*
**************************************************************************/

#ifndef INIFILE_H
#define INIFILE_H

/***********************************
**
**  System Includes
**
***********************************/
#include <windows.h>


/***********************************
**
**  Shared defines
**
***********************************/

#define MAX_MAGCURSOR_STRING      30


/***********************************
**
**  Shared functions
**
***********************************/

void __far __pascal ReadINIMaxHistory( WORD __far * wMaxHistory );

void __far __pascal ReadINIMaxBack( WORD __far * wMaxBack );

BOOL __far __pascal ReadLastMainWndPos( WINDOWPLACEMENT __far * WndPlacePtr );
void __far __pascal WriteLastMainWndPos( WINDOWPLACEMENT __far * WndPlacePtr );

BOOL __far __pascal ReadLastCopyWndPos( WINDOWPLACEMENT __far * WndPlacePtr );
void __far __pascal WriteLastCopyWndPos( WINDOWPLACEMENT __far * WndPlacePtr );

void __far __pascal ReadINIHotSpotColor( COLORREF __far * HotSpotColor );
void __far __pascal ReadScrollBGColor( COLORREF __far * ColorRefPtr );
void __far __pascal ReadNonScrollBGColor( COLORREF __far * ColorRefPtr );

void __far __pascal ReadINIMagCursorName( char __far * szCursorName );

#endif

