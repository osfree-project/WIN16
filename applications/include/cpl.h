/*
 * CPL.H -- Control Panel Applet Interface
 * Совместим с Windows 3.0 и выше
 */

#ifndef CPL_H
#define CPL_H

#include <windows.h>

/* Сообщения для CPlApplet() */
#define CPL_INIT            1
#define CPL_GETCOUNT        2
#define CPL_INQUIRE         3
#define CPL_SELECT          4
#define CPL_DBLCLK          5
#define CPL_STOP            6
#define CPL_EXIT            7
#define CPL_NEWINQUIRE      8   /* только Windows 3.1+, но объявим для совместимости */

/* Структура CPLINFO (Windows 3.0) */
typedef struct tagCPLINFO {
    int     idIcon;     /* идентификатор ресурса иконки */
    int     idName;     /* идентификатор строки с названием */
    int     idInfo;     /* идентификатор строки с описанием */
    LONG    lData;      /* пользовательские данные */
} CPLINFO;
typedef CPLINFO FAR *LPCPLINFO;

/* Структура NEWCPLINFO (Windows 3.1) – для будущей совместимости */
typedef struct tagNEWCPLINFO {
    DWORD   dwSize;
    DWORD   dwFlags;
    DWORD   dwHelpContext;
    LONG    lData;
    HICON   hIcon;
    char    szName[32];
    char    szInfo[64];
    char    szHelpFile[128];
} NEWCPLINFO;
typedef NEWCPLINFO FAR *LPNEWCPLINFO;

/* Прототип функции CPlApplet */
typedef LONG (CALLBACK FAR *CPLAPPLET_PROC)(HWND hwndCPL, UINT msg,
                                             LONG lParam1, LONG lParam2);

#endif
