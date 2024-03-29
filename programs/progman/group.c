/*
 * Program Manager
 *
 * Copyright 1996 Ulrich Schmid
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

#include <stdio.h>
#include <string.h>
#include "win16.h"

#define GlobalPtrHandle(lp) \
  ((HGLOBAL)LOWORD(GlobalHandle(SELECTOROF(lp))))

#define     GlobalUnlockPtr(lp)      \
                GlobalUnlock(GlobalPtrHandle(lp))

#define GlobalFreePtr(lp) \
  (GlobalUnlockPtr(lp),(BOOL)GlobalFree(GlobalPtrHandle(lp)))

#define GlobalAllocPtr(flags, cb) \
  (GlobalLock(GlobalAlloc((flags), (cb))))

#include "progman.h"

/***********************************************************************
 *
 *           GROUP_GroupWndProc
*/

static LRESULT CALLBACK GROUP_GroupWndProc(HWND hWnd, UINT msg,
                                   WPARAM wParam, LPARAM lParam)
{
#ifdef DEBUG
  char DebugBuffer[200];
#endif

  switch (msg)
    {
    case WM_SYSCOMMAND:
      if (wParam == SC_CLOSE) wParam = SC_MINIMIZE;
      break;

    case WM_PAINT: {
      PROGRAM *program;
      PAINTSTRUCT ps;
      HLOCAL hProgram;
      HFONT hFont;
      HDC hdc;
      HLOCAL hGroup;
#ifdef DEBUG
//  wsprintf(DebugBuffer, "hGroup=%04x:%04x\n\r", SELECTOROF(GetWindowLong(hWnd, 0)), OFFSETOF(GetWindowLong(hWnd, 0)));
//  OutputDebugString(DebugBuffer);
#endif
      hdc = BeginPaint(hWnd, &ps);

      SetBkMode(hdc, TRANSPARENT);

      // Select icon font
      hFont = SelectObject(hdc, Globals.hIconFont);

      hGroup = (HLOCAL) GetWindowLong(hWnd, 0);
      for (hProgram = PROGRAM_FirstProgram(hGroup); hProgram;
           hProgram = PROGRAM_NextProgram(hProgram))
      {
        PROGGROUP   *group   = (PROGGROUP   *)LocalLock(hGroup);
        program = (PROGRAM *)LocalLock(hProgram);
        DrawIcon(hdc, program->x, program->y, program->hIcon);
        if (group->hActiveProgram == hProgram)
        {
	   FillRect(hdc, &program->rcTitle, CreateSolidBrush(GetSysColor(COLOR_ACTIVECAPTION)));
        }

        DrawText(hdc, PROGRAM_ProgramName(hProgram), -1, &program->rcTitle, DT_CENTER|DT_WORDBREAK);
        LocalUnlock(hProgram);
        LocalUnlock(hGroup);
      }
 
      // Restore font
      if (hFont) SelectObject(hdc, hFont);
      GetStockObject(WHITE_BRUSH);

      // Restore normal mode
      SetBkMode(hdc, OPAQUE);

      EndPaint(hWnd, &ps);
      return(0);
    }

    case WM_CREATE:
    {
      /* Associate child window data with window */
      SetWindowLong(hWnd, 0, lParam);
#ifdef DEBUG
  wsprintf(DebugBuffer, "hGroup=%04x:%04x\n\r", SELECTOROF(lParam), OFFSETOF(lParam));
  OutputDebugString(DebugBuffer);
#endif

      return 0;
    }

 // ������ ����� ������� ���
    case WM_LBUTTONDOWN:
    {
      WORD xPos, yPos;
      PROGRAM *program;
      HLOCAL hProgram;
      POINT pt;
      RECT rt;
      HLOCAL hGroup;

      // ���࠭塞 ���न���� ����� ���
      xPos   = LOWORD(lParam);
      yPos   = HIWORD(lParam);


      hGroup = (HLOCAL) GetWindowLong(hWnd, 0);
      for (hProgram = PROGRAM_FirstProgram(hGroup); hProgram;
           hProgram = PROGRAM_NextProgram(hProgram))
      {
        program = (PROGRAM *)LocalLock(hProgram);
        pt.x=xPos;
        pt.y=yPos;
        rt.left=program->x;
        rt.top=program->y;
        rt.right=program->x+32;
        rt.bottom=program->y+32;
        if (PtInRect(&program->rcTitle, pt)||PtInRect(&rt, pt)) 
        {
          PROGGROUP   *group   = (PROGGROUP   *)LocalLock(hGroup);
          group->hActiveProgram = hProgram;
          LocalUnlock(hGroup);
          InvalidateRect(hWnd, NULL, TRUE);

		if (Globals.nEditLevel>=2)
		{
			EnableMenuItem(Globals.hFileMenu, PM_NEW, MF_BYCOMMAND | MF_GRAYED);
			EnableMenuItem(Globals.hFileMenu, PM_MOVE, MF_BYCOMMAND | MF_GRAYED);
			EnableMenuItem(Globals.hFileMenu, PM_COPY, MF_BYCOMMAND | MF_GRAYED);
			EnableMenuItem(Globals.hFileMenu, PM_DELETE, MF_BYCOMMAND | MF_GRAYED);
		} else {
			EnableMenuItem(Globals.hFileMenu, PM_MOVE , MF_ENABLED);
			EnableMenuItem(Globals.hFileMenu, PM_COPY , MF_ENABLED);
			EnableMenuItem(Globals.hFileMenu, PM_NEW , MF_ENABLED);
			EnableMenuItem(Globals.hFileMenu, PM_DELETE , MF_ENABLED);
		}
        }
        LocalUnlock(hProgram);
      }

      break;
//      return 0;
    }

   case WM_LBUTTONDBLCLK:
      {
        PROGGROUP   *group   = (PROGGROUP   *)LocalLock((HLOCAL) GetWindowLong(hWnd, 0));
        PROGRAM_ExecuteProgram(group->hActiveProgram);
        return(0);
      }

    case WM_CHILDACTIVATE:
//    case WM_NCLBUTTONDOWN:
      Globals.hActiveGroup = (HLOCAL) GetWindowLong(hWnd, 0);

		if (Globals.nEditLevel>=1)
		{
			EnableMenuItem(Globals.hFileMenu, PM_NEW, MF_BYCOMMAND | MF_GRAYED);
			EnableMenuItem(Globals.hFileMenu, PM_MOVE, MF_BYCOMMAND | MF_GRAYED);
			EnableMenuItem(Globals.hFileMenu, PM_COPY, MF_BYCOMMAND | MF_GRAYED);
			EnableMenuItem(Globals.hFileMenu, PM_DELETE, MF_BYCOMMAND | MF_GRAYED);
		} else {
			EnableMenuItem(Globals.hFileMenu, PM_MOVE , MF_ENABLED);
			EnableMenuItem(Globals.hFileMenu, PM_COPY , MF_ENABLED);
			EnableMenuItem(Globals.hFileMenu, PM_NEW , MF_ENABLED);
			EnableMenuItem(Globals.hFileMenu, PM_DELETE , MF_ENABLED);
		}

		// This is temporary, until implemented
      EnableMenuItem(Globals.hFileMenu, PM_MOVE , MF_GRAYED);
      EnableMenuItem(Globals.hFileMenu, PM_COPY , MF_GRAYED);
      break;
    }
  return(DefMDIChildProc(hWnd, msg, wParam, lParam));
}

/***********************************************************************
 *
 *           GROUP_RegisterGroupWinClass
 */

ATOM GROUP_RegisterGroupWinClass(void)
{
  WNDCLASS class;

  class.style         = CS_HREDRAW | CS_VREDRAW | CS_DBLCLKS;
  class.lpfnWndProc   = GROUP_GroupWndProc;
  class.cbClsExtra    = 0;
  class.cbWndExtra    = sizeof(LONG);
  class.hInstance     = Globals.hInstance;
  class.hIcon         = Globals.hGroupIcon;
  class.hCursor       = LoadCursor (0, IDC_ARROW);
  class.hbrBackground = GetStockObject (WHITE_BRUSH);
  class.lpszMenuName  = 0;
  class.lpszClassName = STRING_GROUP_WIN_CLASS_NAME;

  return RegisterClass(&class);
}

/***********************************************************************
 *
 *           GROUP_NewGroup
 */

VOID GROUP_NewGroup(void)
{
  char szName[MAX_PATHNAME_LEN] = "";
  char szFile[MAX_PATHNAME_LEN] = "";
  OFSTRUCT dummy;

  if (!DIALOG_GroupAttributes(szName, szFile, MAX_PATHNAME_LEN)) return;

  if (OpenFile(szFile, &dummy, OF_EXIST) == HFILE_ERROR)
    {
      /* File doesn't exist */
      HLOCAL hGroup =
        GROUP_AddGroup(szName, szFile, SW_SHOWNORMAL,
                       DEF_GROUP_WIN_XPOS, DEF_GROUP_WIN_YPOS,
                       DEF_GROUP_WIN_WIDTH, DEF_GROUP_WIN_HEIGHT, 0, 0,
                       FALSE);
      if (!hGroup) return;
      GRPFILE_WriteGroupFile(hGroup);
    }
  else /* File exist */
    GRPFILE_ReadGroupFile(szFile);

  /* FIXME Update progman.ini */
}

/***********************************************************************
 *
 *           GROUP_AddGroup
 */

HLOCAL GROUP_AddGroup(LPCSTR lpszName, LPCSTR lpszGrpFile, int nCmdShow,
                      int x, int y, int width, int height,
                      int iconx, int icony,
                      /* FIXME shouldn't be necessary */
                      BOOL bSuppressShowWindow)
{
  PROGGROUP *group, *prior;
  MDICREATESTRUCT cs;
  int    seqnum;
  HLOCAL hPrior, *p;
  HLOCAL hGroup   = LocalAlloc(LMEM_FIXED, sizeof(PROGGROUP));
  HLOCAL hName    = LocalAlloc(LMEM_FIXED, 1 + lstrlen(lpszName));
  HLOCAL hGrpFile = LocalAlloc(LMEM_FIXED, 1 + lstrlen(lpszGrpFile));

#ifdef DEBUG
  char DebugBuffer[200];
  OutputDebugString(__FUNCTION__ " Start\n\r");
#endif

  if (!hGroup || !hName || !hGrpFile)
    {
      MAIN_MessageBoxIDS(IDS_OUT_OF_MEMORY, IDS_ERROR, MB_OK);
      if (hGroup)   LocalFree(hGroup);
      if (hName)    LocalFree(hName);
      if (hGrpFile) LocalFree(hGrpFile);
#ifdef DEBUG
      OutputDebugString("Error allocate memory\n\r");
#endif
      return(0);
    }
  _fmemcpy(LocalLock(hName), lpszName, 1 + lstrlen(lpszName));
  _fmemcpy(LocalLock(hGrpFile), lpszGrpFile, 1 + lstrlen(lpszGrpFile));

  Globals.hActiveGroup   = hGroup;

  seqnum = 1;
  hPrior = 0;
  p = &Globals.hGroups;
  while (*p)
    {
      hPrior = *p;
      prior  = (PROGGROUP *)LocalLock(hPrior);
      p      = &prior->hNext;
      if (prior->seqnum >= seqnum)
        seqnum = prior->seqnum + 1;
    }
  *p = hGroup;

  group = (PROGGROUP *)LocalLock(hGroup);
  group->hPrior    = hPrior;
  group->hNext     = 0;
  group->hName     = hName;
  group->hGrpFile  = hGrpFile;
  group->seqnum    = seqnum;
  group->nCmdShow  = nCmdShow;
  group->x         = x;
  group->y         = y;
  group->width     = width;
  group->height    = height;
  group->iconx     = iconx;
  group->icony     = icony;
  group->hPrograms = 0;
  group->hActiveProgram = 0;

  cs.szClass = STRING_GROUP_WIN_CLASS_NAME;
  cs.szTitle = lpszName;
  cs.hOwner  = Globals.hInstance;
  cs.x       = x;
  cs.y       = y;
  cs.cx      = width;
  cs.cy      = height;
  cs.style   = 0;
  cs.lParam  = (LONG) hGroup;

#ifdef DEBUG
  wsprintf(DebugBuffer, "hGroup=%04x:%04x\n\r", SELECTOROF(hGroup), OFFSETOF(hGroup));
  OutputDebugString(DebugBuffer);
#endif

  group->hWnd = (HWND)SendMessage(Globals.hMDIWnd, WM_MDICREATE, 0, (LPARAM)&cs);

  SetWindowLong(group->hWnd, 0, (UINT) hGroup);
#ifdef DEBUG
//  wsprintf(DebugBuffer, "hGroup=%04x:%04x\n\r", SELECTOROF(GetWindowLong(hWnd, 0)), OFFSETOF(GetWindowLong(hWnd, 0)));
// OutputDebugString(DebugBuffer);
//  if ((UINT)hGroup!=GetWindowLong(group->hWnd, 0)) return(0);
#endif

#if 1
  if (!bSuppressShowWindow) /* FIXME shouldn't be necessary */
#endif
    {
      ShowWindow (group->hWnd, group->nCmdShow);
      UpdateWindow (group->hWnd);
    }

#ifdef DEBUG
      OutputDebugString(__FUNCTION__ "End\n\r");
#endif
  return(hGroup);
}

/***********************************************************************
 *
 *           GROUP_ModifyGroup
 */

VOID GROUP_ModifyGroup(HLOCAL hGroup)
{
  PROGGROUP *group = (PROGGROUP *)LocalLock(hGroup);
  char szName[MAX_PATHNAME_LEN];
  char szFile[MAX_PATHNAME_LEN];
  lstrcpyn(szName, LocalLock(group->hName), MAX_PATHNAME_LEN);
  lstrcpyn(szFile, LocalLock(group->hGrpFile), MAX_PATHNAME_LEN);

  if (!DIALOG_GroupAttributes(szName, szFile, MAX_PATHNAME_LEN)) return;

  //if (strcmp(szFile, LocalLock(group->hGrpFile)))
  //{};

  MAIN_ReplaceString(&group->hName,    szName);
  MAIN_ReplaceString(&group->hGrpFile, szFile);

  GRPFILE_WriteGroupFile(hGroup);

  /* FIXME Delete old GrpFile if GrpFile changed */

  /* FIXME Update progman.ini */

  SetWindowText(group->hWnd, szName);
}

/***********************************************************************
 *
 *           GROUP_ShowGroupWindow
 */

/* FIXME shouldn't be necessary */
VOID GROUP_ShowGroupWindow(HLOCAL hGroup)
{
  PROGGROUP *group = (PROGGROUP *)LocalLock(hGroup);
  ShowWindow (group->hWnd, group->nCmdShow);
  UpdateWindow (group->hWnd);
}

/***********************************************************************
 *
 *           GROUP_DeleteGroup
 */

VOID GROUP_DeleteGroup(HLOCAL hGroup)
{
  PROGGROUP *group = (PROGGROUP *)LocalLock(hGroup);

  Globals.hActiveGroup = 0;

  if (group->hPrior)
    ((PROGGROUP*)LocalLock(group->hPrior))->hNext = group->hNext;
  else Globals.hGroups = group->hNext;

  if (group->hNext)
    ((PROGGROUP*)LocalLock(group->hNext))->hPrior = group->hPrior;

  while (group->hPrograms)
    PROGRAM_DeleteProgram(group->hPrograms, FALSE);

  /* FIXME Update progman.ini */

  SendMessage(Globals.hMDIWnd, WM_MDIDESTROY, (WPARAM)group->hWnd, 0);

  LocalFree(group->hName);
  LocalFree(group->hGrpFile);
  LocalFree(hGroup);
}

/***********************************************************************
 *
 *           GROUP_FirstGroup
 */

HLOCAL GROUP_FirstGroup(void)
{
  return(Globals.hGroups);
}

/***********************************************************************
 *
 *           GROUP_NextGroup
 */

HLOCAL GROUP_NextGroup(HLOCAL hGroup)
{
  PROGGROUP *group;
  if (!hGroup) return(0);
  group = (PROGGROUP *)LocalLock(hGroup);
  return(group->hNext);
}

/***********************************************************************
 *
 *           GROUP_ActiveGroup
 */

HLOCAL GROUP_ActiveGroup(void)
{
  return(Globals.hActiveGroup);
}

/***********************************************************************
 *
 *           GROUP_GroupWnd
 */

HWND GROUP_GroupWnd(HLOCAL hGroup)
{
  PROGGROUP *group;
  if (!hGroup) return(0);
  group = (PROGGROUP *)LocalLock(hGroup);
  return(group->hWnd);
}

/***********************************************************************
 *
 *           GROUP_GroupName
 */

LPCSTR GROUP_GroupName(HLOCAL hGroup)
{
  PROGGROUP *group;
  if (!hGroup) return(0);
  group = (PROGGROUP *)LocalLock(hGroup);
  return(LocalLock(group->hName));
}

/* Local Variables:    */
/* c-file-style: "GNU" */
/* End:                */
