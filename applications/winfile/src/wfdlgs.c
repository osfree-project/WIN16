/********************************************************************

   wfdlgs.c

   Windows File System Dialog procedures

   Copyright (c) Microsoft Corporation. All rights reserved.
   Licensed under the MIT License.

********************************************************************/

#include "winfile.h"
#include <commdlg.h>
#include <dlgs.h>
#include "lfn.h"
#include "wfcopy.h"

VOID  MDIClientSizeChange(HWND hwndActive, INT iFlags);

extern INT maxExt;

VOID
SaveWindows(HWND hwndMain)
{
   // 2* added to both lines
   TCHAR szPath[2*MAXPATHLEN];
   TCHAR buf2[2*MAXPATHLEN + 6*12];

   TCHAR key[10];
   INT dir_num;
   HWND hwnd;
   BOOL bCounting;
   RECT rcT;
   DWORD view, sort, attribs;
   WINDOWPLACEMENT wp;

   // save main window position

   wp.length = sizeof(WINDOWPLACEMENT);
   if (!GetWindowPlacement(hwndMain, &wp))
       return;

   SystemParametersInfo(SPI_GETWORKAREA, 0, (PVOID)&rcT, 0);

   wsprintf(buf2, "%d,%d,%d,%d, , ,%d", rcT.left + wp.rcNormalPosition.left,
      rcT.top + wp.rcNormalPosition.top,
      wp.rcNormalPosition.right - wp.rcNormalPosition.left,
      wp.rcNormalPosition.bottom - wp.rcNormalPosition.top,
      wp.showCmd);

   WritePrivateProfileString(szSettings, szWindow, buf2, szTheINIFile);

   // write out dir window strings in reverse order
   // so that when we read them back in we get the same Z order

   bCounting = TRUE;
   dir_num = 0;

DO_AGAIN:

   for (hwnd = GetWindow(hwndMDIClient, GW_CHILD); hwnd; hwnd = GetWindow(hwnd, GW_HWNDNEXT)) {
      HWND ht = HasTreeWindow(hwnd);
      INT nReadLevel = ht ? GetWindowLong(ht, GWL_READLEVEL) : 0;

      // don't save MDI icon title windows or search windows,
      // or any dir window which is currently recursing

      if ((GetWindow(hwnd, GW_OWNER) == NULL) &&
         GetWindowLong(hwnd, GWL_TYPE) != TYPE_SEARCH) /* nReadLevel == 0) */ {

         if (bCounting) {
            dir_num++;
            continue;
         }

         wp.length = sizeof(WINDOWPLACEMENT);
         if (!GetWindowPlacement(hwnd, &wp))
             continue;
         view = GetWindowLong(hwnd, GWL_VIEW);
         sort = GetWindowLong(hwnd, GWL_SORT);
         attribs = GetWindowLong(hwnd, GWL_ATTRIBS);

         GetMDIWindowText(hwnd, szPath, COUNTOF(szPath));

         wsprintf(key, szDirKeyFormat, dir_num--);

         // format:
         //   x_win, y_win,
         //   x_win, y_win,
         //   x_icon, y_icon,
         //   show_window, view, sort, attribs, split, directory

         wsprintf(buf2, "%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%s",
            wp.rcNormalPosition.left, wp.rcNormalPosition.top,
            wp.rcNormalPosition.right, wp.rcNormalPosition.bottom,
            wp.ptMinPosition.x, wp.ptMinPosition.y,
            wp.showCmd, view, sort, attribs,
            GetSplit(hwnd),
            szPath);

         // the dir is an ANSI string (?)

         WritePrivateProfileString(szSettings, key, buf2, szTheINIFile);
      }
   }

   if (bCounting) {
      bCounting = FALSE;

      // erase the last dir window so that if they save with
      // fewer dirs open we don't pull in old open windows

      wsprintf(key, szDirKeyFormat, dir_num + 1);
      WritePrivateProfileString(szSettings, key, NULL, szTheINIFile);

      goto DO_AGAIN;
   }
}




/*--------------------------------------------------------------------------*/
/*                                                                          */
/*  OtherDlgProc() -                                                        */
/*                                                                          */
/*--------------------------------------------------------------------------*/

BOOL
CALLBACK
OtherDlgProc(register HWND hDlg, UINT wMsg, WPARAM wParam, LPARAM lParam)
{
  DWORD          dwView;
  register HWND hwndActive;

  UNREFERENCED_PARAMETER(lParam);

  hwndActive = (HWND)SendMessage(hwndMDIClient, WM_MDIGETACTIVE, 0, 0L);

  switch (wMsg)
    {
      case WM_INITDIALOG:

          dwView = GetWindowLong(hwndActive, GWL_VIEW);
          CheckDlgButton(hDlg, IDD_SIZE,  dwView & VIEW_SIZE);
          CheckDlgButton(hDlg, IDD_DATE,  dwView & VIEW_DATE);
          CheckDlgButton(hDlg, IDD_TIME,  dwView & VIEW_TIME);
          CheckDlgButton(hDlg, IDD_FLAGS, dwView & VIEW_FLAGS);

          CheckDlgButton(hDlg, IDD_DOSNAMES, dwView & VIEW_DOSNAMES);

          break;

      case WM_COMMAND:
          switch (GET_WM_COMMAND_ID(wParam, lParam))
            {
              case IDD_HELP:
                  goto DoHelp;

              case IDCANCEL:
                  EndDialog(hDlg, FALSE);
                  break;

              case IDOK:
                  {
                  HWND hwnd;

                  dwView = GetWindowLong(hwndActive, GWL_VIEW) & VIEW_PLUSES;

                  if (IsDlgButtonChecked(hDlg, IDD_SIZE))
                        dwView |= VIEW_SIZE;
                  if (IsDlgButtonChecked(hDlg, IDD_DATE))
                        dwView |= VIEW_DATE;
                  if (IsDlgButtonChecked(hDlg, IDD_TIME))
                        dwView |= VIEW_TIME;
                  if (IsDlgButtonChecked(hDlg, IDD_FLAGS))
                        dwView |= VIEW_FLAGS;

                  if (IsDlgButtonChecked(hDlg, IDD_DOSNAMES))
                        dwView |= VIEW_DOSNAMES;

                  EndDialog(hDlg, TRUE);

                  if (hwnd = HasDirWindow(hwndActive))
                      SendMessage(hwnd, FS_CHANGEDISPLAY, CD_VIEW, dwView);
                  else if (hwndActive == hwndSearch) {
                      SetWindowLong(hwndActive, GWL_VIEW, dwView);

                  SendMessage(hwndActive, FS_CHANGEDISPLAY, CD_VIEW, 0L);
                  }

                  break;
                  }

              default:
                  return FALSE;
            }
          break;

       default:

          if (wMsg == wHelpMessage) {
DoHelp:
                WFHelp(hDlg);

                return TRUE;
          } else
                return FALSE;
     }
  return TRUE;
}



/*--------------------------------------------------------------------------*/
/*                                                                          */
/*  IncludeDlgProc() -                                                      */
/*                                                                          */
/*--------------------------------------------------------------------------*/

BOOL
CALLBACK
IncludeDlgProc(HWND hDlg, UINT wMsg, WPARAM wParam, LPARAM lParam)
{
  DWORD dwAttribs;
  HWND hwndActive;

  // To handle LONG paths with LONG filters (illegal)
  TCHAR szTemp[2*MAXPATHLEN];
  TCHAR szInclude[MAXFILENAMELEN];
  HWND hwndDir;

  UNREFERENCED_PARAMETER(lParam);

  hwndActive = (HWND)SendMessage(hwndMDIClient, WM_MDIGETACTIVE, 0, 0L);

  switch (wMsg)
    {
      case WM_INITDIALOG:

          SendMessage(hwndActive, FS_GETFILESPEC, COUNTOF(szTemp), (LPARAM)szTemp);
          SetDlgItemText(hDlg, IDD_NAME, szTemp);
          SendDlgItemMessage(hDlg, IDD_NAME, EM_LIMITTEXT, MAXFILENAMELEN-1, 0L);

          dwAttribs = (DWORD)GetWindowLong(hwndActive, GWL_ATTRIBS);

          CheckDlgButton(hDlg, IDD_DIR,        dwAttribs & ATTR_DIR);
          CheckDlgButton(hDlg, IDD_PROGRAMS,   dwAttribs & ATTR_PROGRAMS);
          CheckDlgButton(hDlg, IDD_DOCS,       dwAttribs & ATTR_DOCS);
          CheckDlgButton(hDlg, IDD_OTHER,      dwAttribs & ATTR_OTHER);
          CheckDlgButton(hDlg, IDD_SHOWHIDDEN, dwAttribs & ATTR_HIDDEN);

          break;

      case WM_COMMAND:
          switch (GET_WM_COMMAND_ID(wParam, lParam))
            {
              case IDD_HELP:
                  goto DoHelp;

              case IDCANCEL:
                  EndDialog(hDlg, FALSE);
                  break;

              case IDOK:

                  GetDlgItemText(hDlg, IDD_NAME, szInclude, COUNTOF(szInclude));

                  // strip out quotes and trailing spaces
                  KillQuoteTrailSpace(szInclude);

                  if (szInclude[0] == 0L)
                     lstrcpy(szInclude, szStarDotStar);

                  dwAttribs = 0;
                  if (IsDlgButtonChecked(hDlg, IDD_DIR))
                      dwAttribs |= ATTR_DIR;
                  if (IsDlgButtonChecked(hDlg, IDD_PROGRAMS))
                      dwAttribs |= ATTR_PROGRAMS;
                  if (IsDlgButtonChecked(hDlg, IDD_DOCS))
                      dwAttribs |= ATTR_DOCS;
                  if (IsDlgButtonChecked(hDlg, IDD_OTHER))
                      dwAttribs |= ATTR_OTHER;
                  if (IsDlgButtonChecked(hDlg, IDD_SHOWHIDDEN))
                      dwAttribs |= ATTR_HS;

                  if (!dwAttribs)
                        dwAttribs = ATTR_EVERYTHING;

                  EndDialog(hDlg, TRUE);        // here to avoid exces repaints

                  // we need to update the tree if they changed the system/hidden
                  // flags.  ...  FIX31

                  if (hwndDir = HasDirWindow(hwndActive)) {
                      SendMessage(hwndDir, FS_GETDIRECTORY, COUNTOF(szTemp), (LPARAM)szTemp);
                      lstrcat(szTemp, szInclude);
                      SetMDIWindowText(hwndActive, szTemp);

                      SetWindowLong(hwndActive, GWL_ATTRIBS, dwAttribs);
                      SendMessage(hwndDir, FS_CHANGEDISPLAY, CD_PATH, 0L);
                  }

                  break;

              default:
                  return FALSE;
            }
          break;

       default:
          if (wMsg == wHelpMessage) {
DoHelp:
                WFHelp(hDlg);

                return TRUE;
          } else
                return FALSE;
     }
  return TRUE;
}


BOOL
CALLBACK
SelectDlgProc(HWND hDlg, UINT wMsg, WPARAM wParam, LPARAM lParam)
{
        HWND hwndActive, hwnd;
        TCHAR szList[128];
        TCHAR szSpec[MAXFILENAMELEN];
        LPTSTR p;

        UNREFERENCED_PARAMETER(lParam);

        switch (wMsg) {
        case WM_INITDIALOG:
                SendDlgItemMessage(hDlg, IDD_NAME, EM_LIMITTEXT, COUNTOF(szList)-1, 0L);
                SetDlgItemText(hDlg, IDD_NAME, szStarDotStar);
                break;

        case WM_COMMAND:
                switch (GET_WM_COMMAND_ID(wParam, lParam)) {
                case IDD_HELP:
                        goto DoHelp;

                case IDCANCEL:
                        EndDialog(hDlg, FALSE);
                        break;

                case IDOK:      // select
                case IDYES:     // unselect

                        // change "Cancel" to "Close"

                        LoadString(hAppInstance, IDS_ANDCLOSE, szSpec, COUNTOF(szSpec));
                        SetDlgItemText(hDlg, IDCANCEL, szSpec);

                        hwndActive = (HWND)SendMessage(hwndMDIClient, WM_MDIGETACTIVE, 0, 0L);

                        if (!hwndActive)
                            break;

                        GetDlgItemText(hDlg, IDD_NAME, szList, COUNTOF(szList));

                        if (hwndActive == hwndSearch)
                            hwnd = hwndSearch;
                        else
                            hwnd = HasDirWindow(hwndActive);

                        if (hwnd) {

                            p = szList;

                            while (p = GetNextFile(p, szSpec, COUNTOF(szSpec)))
                                SendMessage(hwnd, FS_SETSELECTION, (BOOL)(GET_WM_COMMAND_ID(wParam, lParam) == IDOK), (LPARAM)szSpec);
                        }

                        if (hwnd != hwndSearch)
                            UpdateStatus(hwndActive);
                        break;

                default:
                        return FALSE;
                }
                break;

        default:
          if (wMsg == wHelpMessage) {
DoHelp:
                WFHelp(hDlg);

                return TRUE;
          } else
                return FALSE;
        }
        return TRUE;
}


BOOL
CALLBACK
FontHookProc(HWND hDlg, UINT wMsg, WPARAM wParam, LPARAM lParam)
{
KOREAJAPANBEGIN
   // Steal from PBrush source to remove @font in the list - SangilJ
   TCHAR str[LF_FULLFACESIZE], sel[LF_FULLFACESIZE];
   INT index;
   INT cnt;
KOREAJAPANEND

   UNREFERENCED_PARAMETER(lParam);

   switch (wMsg) {
   case WM_INITDIALOG:

      // Delete the Vertical Font Face Name in Font Dialog Box of File Manager  by Sabgilj 01.14.93

      if (bKOREAJAPAN) {

         cnt = (INT)SendDlgItemMessage(hDlg, cmb1, CB_GETCOUNT, 0, 0L);
         index = (INT)SendDlgItemMessage(hDlg, cmb1, CB_GETCURSEL, 0, 0L);
         SendDlgItemMessage(hDlg, cmb1, CB_GETLBTEXT, index, (LPARAM)sel);

         for (index = 0 ; index < cnt ; ) {
            SendDlgItemMessage( hDlg, cmb1, CB_GETLBTEXT, index, (LPARAM)str);
            if (str[0] == '@')
               cnt = (INT)SendDlgItemMessage(hDlg,cmb1,CB_DELETESTRING,index,0L);
            else
               index++;
         }
         index = (INT)SendDlgItemMessage(hDlg, cmb1, CB_FINDSTRING, (WPARAM)-1, (LPARAM)sel);
         SendDlgItemMessage(hDlg, cmb1, CB_SETCURSEL, index, 0L);

      }
      CheckDlgButton(hDlg, chx3, wTextAttribs & TA_LOWERCASE);

      CheckDlgButton(hDlg, chx4, wTextAttribs & TA_LOWERCASEALL);
      break;

   case WM_COMMAND:
      switch (wParam) {
      case pshHelp:
         SendMessage(hwndFrame, wHelpMessage, 0, 0L);
         break;

      case IDOK:
         if (IsDlgButtonChecked(hDlg, chx3))
            wTextAttribs |= TA_LOWERCASE;
         else
            wTextAttribs &= ~TA_LOWERCASE;

         if (IsDlgButtonChecked(hDlg, chx4))
            wTextAttribs |= TA_LOWERCASEALL;
         else
            wTextAttribs &= ~TA_LOWERCASEALL;
         break;
      }
   }
   return FALSE;
}

VOID
RepaintDrivesForFontChange(HWND hwndChild)
{
   if (bDriveBar)
      MDIClientSizeChange(hwndChild,DRIVEBAR_FLAG);
}

VOID
NewFont()
{
   HFONT hOldFont;
   HANDLE hOld;

   HWND hwnd, hwndT, hwndT2;
   HDC hdc;
   LOGFONT lf;
   CHOOSEFONT cf;
   TCHAR szBuf[10];
   INT res;
   UINT uOld,uNew;

#define MAX_PT_SIZE 36

   GetObject(hFont, sizeof(lf), (LPVOID)(LPLOGFONT)&lf);

   //
   // As we use 'system' font as default, and set initial size 0 for logfont so
   // that we can get default size on system, we may haven't got real font
   // height yet. mskk.
   //
   if (bJAPAN && lf.lfHeight == 0) {
      TEXTMETRIC tm;

      hdc = GetDC(NULL);
      hOld = SelectObject(hdc, hFont);
      GetTextMetrics(hdc,&tm);
      if (hOld)
         SelectObject(hdc, hOld);
      ReleaseDC(NULL, hdc);
      lf.lfHeight = -(tm.tmHeight-tm.tmInternalLeading);
   }

   uOld = (UINT)abs(lf.lfHeight);

   cf.lStructSize    = sizeof(cf);
   cf.hwndOwner      = hwndFrame;
   cf.lpLogFont      = &lf;
   cf.hInstance      = hAppInstance;
   cf.lpTemplateName = (LPTSTR) MAKEINTRESOURCE(FONTDLG);
   cf.lpfnHook       = FontHookProc;
   cf.nSizeMin       = 4;
   cf.nSizeMax       = 36;

   cf.Flags          = bJAPAN ?
                          CF_SCREENFONTS | CF_SHOWHELP |
                             CF_ENABLEHOOK | CF_ENABLETEMPLATE |
                             CF_INITTOLOGFONTSTRUCT | CF_LIMITSIZE :
                          CF_SCREENFONTS | CF_SHOWHELP |
                             CF_ENABLEHOOK | CF_ENABLETEMPLATE |
                             CF_INITTOLOGFONTSTRUCT | CF_LIMITSIZE |
                             CF_ANSIONLY;

   if (!LoadComdlg()) {
      return;
   }

   res = ChooseFontW(&cf);

   if (!res)
      return;

   wsprintf(szBuf, SZ_PERCENTD, cf.iPointSize / 10);

   uNew = (UINT)abs(lf.lfHeight);

   if (bJAPAN && lf.lfCharSet != SHIFTJIS_CHARSET)
      MyMessageBox(hwndFrame, IDS_WINFILE, IDS_WRNNOSHIFTJIS,
                                             MB_OK|MB_ICONEXCLAMATION);

   // Set wTextAttribs BOLD and ITALIC flags

   if (lf.lfItalic != 0)
      wTextAttribs |= TA_ITALIC;
   else
      wTextAttribs &= ~TA_ITALIC;

   WritePrivateProfileString(szSettings, szFace, lf.lfFaceName, szTheINIFile);
   WritePrivateProfileString(szSettings, szSize, szBuf, szTheINIFile);
   WritePrivateProfileBool(szLowerCase, wTextAttribs);
   WritePrivateProfileBool(szFaceWeight, lf.lfWeight);

   if (bJAPAN)
      WritePrivateProfileBool(szSaveCharset, lf.lfCharSet);

   hOldFont = hFont;

   hFont = CreateFontIndirect(&lf);

   if (!hFont) {
      DeleteObject(hOldFont);
      return;
   }

   // recalc all the metrics for the new font

   hdc = GetDC(NULL);
   hOld = SelectObject(hdc, hFont);
   GetTextStuff(hdc);
   if (hOld)
      SelectObject(hdc, hOld);
   ReleaseDC(NULL, hdc);


   RepaintDrivesForFontChange((HWND)SendMessage(hwndMDIClient, WM_MDIGETACTIVE, 0, 0L));

   // now update all listboxes that are using the old
   // font with the new font

   for (hwnd = GetWindow(hwndMDIClient, GW_CHILD); hwnd;
      hwnd = GetWindow(hwnd, GW_HWNDNEXT)) {

      if (GetWindow(hwnd, GW_OWNER))
         continue;

      if ((INT)GetWindowLong(hwnd, GWL_TYPE) == TYPE_SEARCH) {
         SendMessage((HWND)GetDlgItem(hwnd, IDCW_LISTBOX), WM_SETFONT, (WPARAM)hFont, MAKELPARAM(TRUE, 0));
         SendMessage((HWND)GetDlgItem(hwnd, IDCW_LISTBOX), LB_SETITEMHEIGHT, 0, (LONG)dyFileName);

         // SearchWin font ext
         // in case font changed, update maxExt

         SendMessage(hwnd, FS_CHANGEDISPLAY, CD_SEARCHFONT, 0L);

      } else {

         if (hwndT = HasDirWindow(hwnd)) {

            hwndT2 = GetDlgItem(hwndT, IDCW_LISTBOX);
            SetLBFont(hwndT,
                      hwndT2,
                      hFont,
                      GetWindowLong(hwnd, GWL_VIEW),
                      (LPXDTALINK)GetWindowLong(hwndT, GWL_HDTA));

            InvalidateRect(hwndT2, NULL, TRUE);
         }

         if (hwndT = HasTreeWindow(hwnd)) {

            // the tree list box

            hwndT = GetDlgItem(hwndT, IDCW_TREELISTBOX);

            SendMessage(hwndT, WM_SETFONT, (WPARAM)hFont, MAKELPARAM(TRUE, 0));
            SendMessage(hwndT, LB_SETITEMHEIGHT, 0, (LONG)dyFileName);

            /*
             *  Force the recalculation of GWL_XTREEMAX (max text extent).
             */
            SendMessage(HasTreeWindow(hwnd), TC_RECALC_EXTENT, (WPARAM)hwndT, 0L);
         }
      }
   }
   DeleteObject(hOldFont); // done with this now, delete it
}



/*--------------------------------------------------------------------------*/
/*                                                                          */
/*  ConfirmDlgProc() -                                                      */
/*                                                                          */
/*--------------------------------------------------------------------------*/

BOOL
CALLBACK
ConfirmDlgProc(HWND hDlg, UINT wMsg, WPARAM wParam, LPARAM lParam)
{
  UNREFERENCED_PARAMETER(lParam);

  switch (wMsg)
    {
      case WM_INITDIALOG:
          CheckDlgButton(hDlg, IDD_DELETE,  bConfirmDelete);
          CheckDlgButton(hDlg, IDD_SUBDEL,  bConfirmSubDel);
          CheckDlgButton(hDlg, IDD_REPLACE, bConfirmReplace);
          CheckDlgButton(hDlg, IDD_MOUSE,   bConfirmMouse);
          CheckDlgButton(hDlg, IDD_CONFIG,  bConfirmFormat);
          CheckDlgButton(hDlg, IDD_READONLY,bConfirmReadOnly);
          break;

      case WM_COMMAND:
          switch (GET_WM_COMMAND_ID(wParam, lParam))
            {
              case IDD_HELP:
                  goto DoHelp;

              case IDCANCEL:
                  EndDialog(hDlg, FALSE);
                  break;

              case IDOK:
                  bConfirmDelete  = IsDlgButtonChecked(hDlg, IDD_DELETE);
                  bConfirmSubDel  = IsDlgButtonChecked(hDlg, IDD_SUBDEL);
                  bConfirmReplace = IsDlgButtonChecked(hDlg, IDD_REPLACE);
                  bConfirmMouse   = IsDlgButtonChecked(hDlg, IDD_MOUSE);
                  bConfirmFormat  = IsDlgButtonChecked(hDlg, IDD_CONFIG);

                  bConfirmReadOnly= IsDlgButtonChecked(hDlg, IDD_READONLY);

                  WritePrivateProfileBool(szConfirmDelete,  bConfirmDelete);
                  WritePrivateProfileBool(szConfirmSubDel,  bConfirmSubDel);
                  WritePrivateProfileBool(szConfirmReplace, bConfirmReplace);
                  WritePrivateProfileBool(szConfirmMouse,   bConfirmMouse);
                  WritePrivateProfileBool(szConfirmFormat,  bConfirmFormat);

                  WritePrivateProfileBool(szConfirmReadOnly,bConfirmReadOnly);

                  EndDialog(hDlg, TRUE);
                  break;

              default:
                  return(FALSE);
            }
          break;

       default:
          if (wMsg == wHelpMessage) {
DoHelp:
                WFHelp(hDlg);

                return TRUE;
          } else
                return FALSE;
     }
  return TRUE;
}

VOID
KillQuoteTrailSpace( LPTSTR szFile )
{
   register LPTSTR pc;
   register LPTSTR pcNext;
   LPTSTR pcLastSpace = NULL;

   // Could reuse szFile, but that's ok,
   // we use it as a register probably anyway.

   pc = pcNext = szFile;

   while ( *pcNext ) {

      if ( CHAR_DQUOTE == *pcNext ) {
         pcNext++;
         continue;
      }

      if ( CHAR_SPACE == *pcNext ) {
         if ( !pcLastSpace ) {
            pcLastSpace = pc;
         }
      } else {
         pcLastSpace = NULL;
      }

      *pc++ = *pcNext++;
   }

   // Delimit!
   *pc = CHAR_NULL;

   // Now axe trailing spaces;
   if (pcLastSpace)
      *pcLastSpace = CHAR_NULL;
}
