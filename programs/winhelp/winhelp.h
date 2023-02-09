/*
 * Help Viewer
 *
 * Copyright    1996 Ulrich Schmid
 * Copyright    2002 Sylvain Petreolle <spetreolle@yahoo.fr>
 *              2002 Eric Pouech
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
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#define MAX_LANGUAGE_NUMBER     255
#define MAX_PATHNAME_LEN        1024
#define MAX_STRING_LEN          255

#define INTERNAL_BORDER_WIDTH   5
#define POPUP_YDISTANCE         20
#define SHADOW_DX               10
#define SHADOW_DY               10
#define BUTTON_CX               6
#define BUTTON_CY               6

#ifndef RC_INVOKED

#include <stdarg.h>

#include <windows.h>

#include "hlpfile.h"
#include "macro.h"
#include "winhelp_res.h"

int WINAPI ShellAbout(HWND hWnd, LPCSTR lpszCaption, LPCSTR lpszAboutText,
                HICON hIcon);

typedef struct tagHelpLinePart
{
    RECT      rect;
    enum {hlp_line_part_text, hlp_line_part_bitmap, hlp_line_part_metafile} cookie;
    union
    {
        struct
        {
            LPCSTR      lpsText;
            HFONT       hFont;
            COLORREF    color;
            WORD        wTextLen;
            WORD        wUnderline; /* 0 None, 1 simple, 2 double, 3 dotted */
        } text;
        struct
        {
            HBITMAP     hBitmap;
        } bitmap;
        struct
        {
            HMETAFILE   hMetaFile;
        } metafile;
    } u;
    HLPFILE_LINK far *       link;

    struct tagHelpLinePart far *next;
} WINHELP_LINE_PART;

typedef struct tagHelpLine
{
    RECT                rect;
    WINHELP_LINE_PART   first_part;
    struct tagHelpLine far * next;
} WINHELP_LINE;

typedef struct tagHelpButton
{
    HWND                hWnd;

    LPCSTR              lpszID;
    LPCSTR              lpszName;
    LPCSTR              lpszMacro;

    WPARAM              wParam;

    RECT                rect;

    struct tagHelpButton*next;
} WINHELP_BUTTON;

typedef struct tagWinHelp
{
    LPCSTR              lpszName;

    WINHELP_BUTTON*     first_button;
    HLPFILE_PAGE*       page;
    WINHELP_LINE far *       first_line;

    HWND                hMainWnd;
    HWND                hButtonBoxWnd;
    HWND                hTextWnd;
    HWND                hShadowWnd;
    HWND                hHistoryWnd;

    HFONT*              fonts;
    UINT                fonts_len;

    HCURSOR             hArrowCur;
    HCURSOR             hHandCur;

    HLPFILE_WINDOWINFO* info;

    /* FIXME: for now it's a fixed size */
    HLPFILE_PAGE*       history[40];
    unsigned            histIndex;
    HLPFILE_PAGE*       back[40];
    unsigned            backIndex;

    struct tagWinHelp far * next;
} WINHELP_WINDOW;

#define DC_NOMSG     0x00000000
#define DC_MINMAX    0x00000001
#define DC_INITTERM  0x00000002
#define DC_JUMP      0x00000004
#define DC_ACTIVATE  0x00000008
#define DC_CALLBACKS 0x00000010

#define DW_NOTUSED    0
#define DW_WHATMSG    1
#define DW_MINMAX     2
#define DW_SIZE       3
#define DW_INIT       4
#define DW_TERM       5
#define DW_STARTJUMP  6
#define DW_ENDJUMP    7
#define DW_CHGFILE    8
#define DW_ACTIVATE   9
#define	DW_CALLBACKS 10

typedef long (CALLBACK *WINHELP_LDLLHandler)(WORD, LONG, LONG);

typedef struct tagDll
{
    HANDLE              hLib;
    const char*         name;
    WINHELP_LDLLHandler handler;
    DWORD               class;
    struct tagDll*      next;
} WINHELP_DLL;

typedef struct
{
    UINT                wVersion;
    HANDLE              hInstance;
    HWND                hPopupWnd;
    UINT                wStringTableOffset;
    BOOL                isBook;
    WINHELP_WINDOW far *active_win;
    WINHELP_WINDOW far *win_list;
    WNDPROC             button_proc;
    WINHELP_DLL*        dlls;
} WINHELP_GLOBALS;

extern WINHELP_GLOBALS Globals;
extern FARPROC         Callbacks[];

BOOL WINHELP_CreateHelpWindowByHash(HLPFILE far *, LONG, HLPFILE_WINDOWINFO*, int);
BOOL WINHELP_CreateHelpWindow(HLPFILE_PAGE*, HLPFILE_WINDOWINFO*, int);
int  WINHELP_MessageBoxIDS(UINT, UINT, WORD);
int  WINHELP_MessageBoxIDS_s(UINT, LPCSTR, UINT, WORD);
HLPFILE far * WINHELP_LookupHelpFile(LPCSTR lpszFile);
HLPFILE_WINDOWINFO* WINHELP_GetWindowInfo(HLPFILE* hlpfile, LPCSTR name);

extern const char MAIN_WIN_CLASS_NAME[];
extern const char BUTTON_BOX_WIN_CLASS_NAME[];
extern const char TEXT_WIN_CLASS_NAME[];
extern const char SHADOW_WIN_CLASS_NAME[];
extern const char HISTORY_WIN_CLASS_NAME[];
extern const char STRING_BUTTON[];
extern const char STRING_MENU_Xx[];
extern const char STRING_DIALOG_TEST[];
#endif


/* Buttons */
#define WH_FIRST_BUTTON     500
