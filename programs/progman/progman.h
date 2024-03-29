/*
 * Program Manager
 *
 * Copyright 1996 Ulrich Schmid
 * Copyright 2002 Sylvain Petreolle
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

#ifndef PROGMAN_H
#define PROGMAN_H

#define MAX_STRING_LEN      255
#define MAX_PATHNAME_LEN    1024
#define MAX_LANGUAGE_NUMBER (PM_LAST_LANGUAGE - PM_FIRST_LANGUAGE)

#include <win16.h>

//int WINAPI GetOpenFileName(LPOPENFILENAME);

DWORD WINAPI GetCurrentDirectory(DWORD cchCurDir, char * lpszCurDir);
HICON WINAPI ExtractIcon(HINSTANCE hInst, LPCSTR lpszExeFileName, UINT nIconIndex);

#define IDI_WINLOGO        MAKEINTRESOURCE(32517)
#define OIC_WINLOGO         32517
#define MAX_PATH        255

typedef struct tagCURSORICONINFO
{
    POINT ptHotSpot;
    WORD    nWidth;
    WORD    nHeight;
    WORD    nWidthBytes;
    BYTE    bPlanes;
    BYTE    bBitsPerPixel;
} CURSORICONINFO;

#define WM_PAINTICON            0x0026

int WINAPI ShellAbout(HWND hWnd, LPCSTR lpszCaption, LPCSTR lpszAboutText,
                HICON hIcon);

/* Fallback icon */
#define DEFAULTICON OIC_WINLOGO

/* Icon index in M$ Window's progman.exe  */
#define PROGMAN_ICON_INDEX 0
#define GROUP_ICON_INDEX   6
#define DEFAULT_ICON_INDEX 7
#define MSDOS_ICON_INDEX 9

#define DEF_GROUP_WIN_XPOS   100
#define DEF_GROUP_WIN_YPOS   100
#define DEF_GROUP_WIN_WIDTH  300
#define DEF_GROUP_WIN_HEIGHT 200

typedef struct
{
  HLOCAL   hGroup;
  HLOCAL   hPrior;
  HLOCAL   hNext;
  HWND     hWnd;
  /**/              /* Numbers are byte indexes in *.grp */

  /**/                       /* Program entry */
  int      x, y;               /*  0 -  3 */
  int      nIconIndex;         /*  4 -  5 */
  HICON    hIcon;
  RECT     rcTitle;		// Icon title rectangle

  /* icon flags ??? */         /*  6 -  7 */
  /* iconANDsize */            /*  8 -  9 */
  /* iconXORsize */            /* 10 - 11 */
  /* pointer to IconInfo    */ /* 12 - 13 */
  /* pointer to iconXORbits */ /* 14 - 15 */ /* sometimes iconANDbits ?! */
  /* pointer to iconANDbits */ /* 16 - 17 */ /* sometimes iconXORbits ?! */
  HLOCAL   hName;              /* 18 - 19 */
  HLOCAL   hCmdLine;           /* 20 - 21 */
  HLOCAL   hIconFile;          /* 22 - 23 */
  HLOCAL   hWorkDir;           /* Extension 0x8101 */
  int      nHotKey;            /* Extension 0x8102 */
  /* Modifier: bit 8... */
  int      nCmdShow;           /* Extension 0x8103 */

  /**/                         /* IconInfo */
  /* HotSpot x   ??? */        /*  0 -  1 */
  /* HotSpot y   ??? */        /*  2 -  3 */
  /* Width           */        /*  4 -  5 */
  /* Height          */        /*  6 -  7 */
  /* WidthBytes  ??? */        /*  8 -  9 */
  /* Planes          */        /* 10 - 10 */
  /* BitsPerPixel    */        /* 11 - 11 */
} PROGRAM;

/* This is in-memory program group structure */
typedef struct
{
  HLOCAL   hPrior;
  HLOCAL   hNext;
  HWND     hWnd;
  HLOCAL   hGrpFile;
  HLOCAL   hActiveProgram;
  int      seqnum;

  /* */                         /* Absolute */
  /* magic `PMCC'  */          /*  0 -  3 */
  /* checksum      */          /*  4 -  5 */
  /* Extension ptr */          /*  6 -  7 */
  int      nCmdShow;           /*  8 -  9 */
  int      x, y;               /* 10 - 13 */
  int      width, height;      /* 14 - 17 */
  int      iconx, icony;       /* 18 - 21 */
  HLOCAL   hName;              /* 22 - 23 */
  /* unknown */                /* 24 - 31 */
  /* number of programs */     /* 32 - 33 */
  HLOCAL   hPrograms;          /* 34 ...  */

  /**/                        /* Extensions */
  /* Extension type */         /*  0 -  1 */
  /* Program number */         /*  2 -  3 */
  /* Size of entry  */         /*  4 -  5 */
  /* Data           */         /*  6 ...  */

  /* magic `PMCC' */           /* Extension 0x8000 */
  /* End of Extensions */      /* Extension 0xffff */
} PROGGROUP;

typedef struct
{
  HANDLE  hInstance;
  HANDLE  hAccel;
  HWND    hMainWnd;
  HWND    hMDIWnd;
  HICON   hMainIcon;
  HICON   hGroupIcon;
  HICON   hDefaultIcon;
  HMENU   hMainMenu;
  HMENU   hFileMenu;
  HMENU   hOptionMenu;
  HMENU   hWindowsMenu;
  LPCSTR  lpszIniFile;			// Name of INI file
  LPCSTR  lpszIcoFile;			// Name of Icon file
  LPCSTR  lpszHlpFile;			// Name of HLP file
  HFONT   hIconFont;			// Font for Icon titles
  int     cxSpacing, cySpacing;		// Icon spacing
  int     cyBorder;			// Border size
  int     cxOffset;			// Offset
  int     cyOffset;			// Offset
  int     cxIconSpace;
  int     cyIconSpace;
  BOOL    bAutoArrange;
  BOOL    bSaveSettings;		// Save settings on exit
  BOOL    bMinOnRun;
  BOOL    bNoRun;			// Disable Run menu item
  BOOL    bNoClose;			// Prevent progman close
  BOOL    bNoSaveSettings;		// Disable Save Settings
  BOOL    bNoFileMenu;			// Remove File Menu
  int     nEditLevel;			// Level of restrictions
  WORD    wLogPixelsX;			// Current display X resolution
  WORD    wLogPixelsY;			// Current display Y resolution
  BYTE    bPlanes;			// Current count of planes
  BYTE    bBitsPixel;			// Current bits per pixel
  HLOCAL  hGroups;
  HLOCAL  hActiveGroup;
} GLOBALS;

extern GLOBALS Globals;

int  MAIN_MessageBoxIDS(UINT ids_text, UINT ids_title, WORD type);
int  MAIN_MessageBoxIDS_s(UINT ids_text_s, LPCSTR str, UINT ids_title, WORD type);
VOID MAIN_ReplaceString(HLOCAL *handle, LPSTR replacestring);

HLOCAL GRPFILE_ReadGroupFile(LPCSTR path);
BOOL   GRPFILE_WriteGroupFile(HLOCAL hGroup);

ATOM   GROUP_RegisterGroupWinClass(void);
HLOCAL GROUP_AddGroup(LPCSTR lpszName, LPCSTR lpszGrpFile, int showcmd,
                      int x, int y, int width, int heiht,
                      int iconx, int icony,
                      /* FIXME shouldn't be necessary */
                      BOOL bSuppressShowWindow);
VOID   GROUP_NewGroup(void);
VOID   GROUP_ModifyGroup(HLOCAL hGroup);
VOID   GROUP_DeleteGroup(HLOCAL hGroup);
/* FIXME shouldn't be necessary */
VOID   GROUP_ShowGroupWindow(HLOCAL hGroup);
HLOCAL GROUP_FirstGroup(void);
HLOCAL GROUP_NextGroup(HLOCAL hGroup);
HLOCAL GROUP_ActiveGroup(void);
HWND   GROUP_GroupWnd(HLOCAL hGroup);
LPCSTR GROUP_GroupName(HLOCAL hGroup);

ATOM   PROGRAM_RegisterProgramWinClass(void);
HLOCAL PROGRAM_AddProgram(HLOCAL hGroup, HICON hIcon, LPCSTR lpszName,
                          int x, int y, LPCSTR lpszCmdLine,
                          LPCSTR lpszIconFile, int nIconIndex,
                          LPCSTR lpszWorkDir, int nHotKey, int nCmdShow);
VOID   PROGRAM_NewProgram(HLOCAL hGroup);
VOID   PROGRAM_ModifyProgram(HLOCAL hProgram);
VOID   PROGRAM_CopyMoveProgram(HLOCAL hProgram, BOOL bMove);
VOID   PROGRAM_DeleteProgram(HLOCAL hProgram, BOOL BUpdateGrpFile);
HLOCAL PROGRAM_FirstProgram(HLOCAL hGroup);
HLOCAL PROGRAM_NextProgram(HLOCAL hProgram);
HLOCAL PROGRAM_ActiveProgram(HLOCAL hGroup);
LPCSTR PROGRAM_ProgramName(HLOCAL hProgram);
VOID   PROGRAM_ExecuteProgram(HLOCAL hLocal);

int    DIALOG_New(int nDefault);
HLOCAL DIALOG_CopyMove(LPCSTR lpszProgramName, LPCSTR lpszGroupName, BOOL bMove);
BOOL   DIALOG_Delete(UINT ids_format_s, LPCSTR lpszName);
BOOL   DIALOG_GroupAttributes(LPSTR lpszTitle, LPSTR lpszPath, int nSize);
BOOL   DIALOG_ProgramAttributes(LPSTR lpszTitle, LPSTR lpszCmdLine,
                                LPSTR lpszWorkDir, LPSTR lpszIconFile,
                                HICON *lphIcon, int *nIconIndex,
                                int *lpnHotKey, int *lpnCmdShow, int nSize);
VOID   DIALOG_Symbol(HICON *lphIcon, LPSTR lpszIconFile,
                     int *lpnIconIndex, int nSize);
VOID   DIALOG_Execute(void);

VOID   STRING_LoadMenus(VOID);

/* Class names */
extern char STRING_MAIN_WIN_CLASS_NAME[];
extern char STRING_MDI_WIN_CLASS_NAME[];
extern char STRING_GROUP_WIN_CLASS_NAME[];
extern char STRING_PROGRAM_WIN_CLASS_NAME[];

/* Resource names */
extern char STRING_ACCEL[];
extern char STRING_MAIN[];
extern char STRING_NEW[];
extern char STRING_OPEN[];
extern char STRING_MOVE[];
extern char STRING_COPY[];
extern char STRING_DELETE[];
extern char STRING_GROUP[];
extern char STRING_PROGRAM[];
extern char STRING_SYMBOL[];
extern char STRING_EXECUTE[];

/* Stringtable index */
#define IDS_PROGRAM_MANAGER            0x02
#define IDS_ERROR                      0x03
#define IDS_WARNING                    0x04
#define IDS_INFO                       0x05
#define IDS_DELETE                     0x06
#define IDS_DELETE_GROUP_s             0x07
#define IDS_DELETE_PROGRAM_s           0x08
#define IDS_NOT_IMPLEMENTED            0x09
#define IDS_FILE_READ_ERROR_s          0x0a
#define IDS_FILE_WRITE_ERROR_s         0x0b
#define IDS_GRPFILE_READ_ERROR_s       0x0c
#define IDS_OUT_OF_MEMORY              0x0d
#define IDS_WINHELP_ERROR              0x0e
#define IDS_UNKNOWN_FEATURE_s          0x0f
#define IDS_FILE_NOT_OVERWRITTEN_s     0x10
#define IDS_SAVE_GROUP_AS_s            0x11
#define IDS_NO_HOT_KEY                 0x12
#define IDS_ALL_FILES                  0x13
#define IDS_PROGRAMS                   0x14
#define IDS_LIBRARIES_DLL              0x15
#define IDS_SYMBOL_FILES               0x16
#define IDS_SYMBOLS_ICO                0x17
#define IDS_GRPFILE_NOT_IN_INI_s		0x18

#define IDS_LICENSE_CAPTION            0x20
#define IDS_LICENSE                    0x21
#define IDS_WARRANTY_CAPTION           0x22
#define IDS_WARRANTY                   0x23

/* Menu */

#define MAIN_MENU           0x109
#define PM_NEW              0x100
#define PM_OPEN             0x101
#define PM_MOVE             0x102
#define PM_COPY             0x103
#define PM_DELETE           0x104
#define PM_ATTRIBUTES       0x105
#define PM_EXECUTE          0x107
#define PM_EXIT             0x108

#define PM_AUTO_ARRANGE     0x110
#define PM_MIN_ON_RUN       0x111
#define PM_SAVE_SETTINGS    0x113

#define PM_OVERLAP          0x120
#define PM_SIDE_BY_SIDE     0x121
#define PM_ARRANGE          0x122
#define PM_FIRST_CHILD      0x3030

/*
 *#define PM_FIRST_LANGUAGE   0x400
 *#define PM_LAST_LANGUAGE    0x499
 */

#define PM_CONTENTS         0x131
#define PM_SEARCH           0x132
#define PM_HELPONHELP       0x133
#define PM_TUTORIAL         0x134

#define PM_LICENSE          0x140
#define PM_NO_WARRANTY      0x141
#define PM_ABOUT_WINE       0x142

/* Dialog `New' */

/* RADIOBUTTON: The next two must be in sequence */
#define PM_NEW_GROUP        0x150
#define PM_NEW_PROGRAM      0x151
#define PM_NEW_GROUP_TXT    0x152
#define PM_NEW_PROGRAM_TXT  0x153

/* Dialogs `Copy', `Move' */

#define PM_PROGRAM          0x160
#define PM_FROM_GROUP       0x161
#define PM_TO_GROUP         0x162
#define PM_TO_GROUP_TXT     0x163

/* Dialogs `Group attributes' */

#define PM_DESCRIPTION      0x170
#define PM_DESCRIPTION_TXT  0x171
#define PM_FILE             0x172
#define PM_FILE_TXT         0x173

/* Dialogs `Program attributes' */
#define PM_COMMAND_LINE     0x180
#define PM_COMMAND_LINE_TXT 0x181
#define PM_DIRECTORY        0x182
#define PM_DIRECTORY_TXT    0x183
#define PM_HOT_KEY          0x184
#define PM_HOT_KEY_TXT      0x185
#define PM_ICON             0x186
#define PM_OTHER_SYMBOL     0x187

/* Dialog `Symbol' */

#define PM_ICON_FILE        0x190
#define PM_ICON_FILE_TXT    0x191
#define PM_SYMBOL_LIST      0x192
#define PM_SYMBOL_LIST_TXT  0x193

/* Dialog `Execute' */

#define PM_COMMAND          0x1a0
#define PM_SYMBOL           0x1a1
#define PM_BROWSE           0x1a2
#define PM_HELP             0x1a3

#endif /* PROGMAN_H */

/* Local Variables:    */
/* c-file-style: "GNU" */
/* End:                */
