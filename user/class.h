/*
 * Window classes definitions
 *
 * Copyright 1993 Alexandre Julliard
 */

#ifndef CLASS_H
#define CLASS_H

#include "user.h"

#define CLASS_MAGIC   0x4b4e      /* 'NK' */

#pragma pack(1)

#define HCLASS HANDLE

/* !! Don't change this structure (see GetClassLong()) */
typedef struct tagCLASS
{
    HCLASS       hNext;         /* Next class */
    WORD         wMagic;        /* Magic number (must be CLASS_MAGIC) */
    ATOM         atomName;      /* Name of the class */
    HANDLE       hdce;          /* Class DCE (if CS_CLASSDC) */
    WORD         cWindows;      /* Count of existing windows of this class */
    WNDCLASS     wc;		/* Class information */
    WORD         wExtra[1];     /* Class extra bytes */
} CLASS;

typedef struct tagWND
{
    struct tagWND *next;         /* Next sibling */
    struct tagWND *child;        /* First child */
    struct tagWND *parent;       /* Window parent (from CreateWindow) */
    struct tagWND *owner;        /* Window owner */
    DWORD        dwMagic;        /* Magic number (must be WND_MAGIC) */
    HWND         hwndSelf;       /* Handle of this window */
    HCLASS       hClass;         /* Window class */
    HANDLE       hInstance;      /* Window hInstance (from CreateWindow) */
    RECT         rectClient;     /* Client area rel. to parent client area */
    RECT         rectWindow;     /* Whole window rel. to parent client area */
    RECT         rectNormal;     /* Window rect. when in normal state */
    POINT        ptIconPos;      /* Icon position */
    POINT        ptMaxPos;       /* Maximized window position */
    HGLOBAL      hmemTaskQ;      /* Task queue global memory handle */
    HRGN         hrgnUpdate;     /* Update region */
    HWND         hwndLastActive; /* Last active popup hwnd */
    WNDPROC      lpfnWndProc;    /* Window procedure */
    DWORD        dwStyle;        /* Window style (from CreateWindow) */
    DWORD        dwExStyle;      /* Extended style (from CreateWindowEx) */
    HANDLE       hdce;           /* Window DCE (if CS_OWNDC or CS_CLASSDC) */
    HANDLE       hVScroll;       /* Vertical scroll-bar info */
    HANDLE       hHScroll;       /* Horizontal scroll-bar info */
    UINT         wIDmenu;        /* ID or hmenu (from CreateWindow) */
    HANDLE       hText;          /* Handle of window text */
    WORD         flags;          /* Misc. flags (see below) */
//    Window       window;         /* X window (only for top-level windows) */
    HMENU        hSysMenu;	 /* window's copy of System Menu */
    HANDLE       hProp;          /* Handle of Properties List */
    WORD         wExtra[1];      /* Window extra bytes */
} WND;

#pragma pack(pop)


extern void CLASS_DumpClass( HCLASS hClass );
extern void CLASS_WalkClasses(void);
extern HCLASS CLASS_FindClassByName( LPCSTR name, HINSTANCE hinstance,
                                     CLASS **ptr );
extern CLASS * CLASS_FindClassPtr( HCLASS hclass );

#endif  /* CLASS_H */
