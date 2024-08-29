/********************************************************************

   wfDirSrc.c

   Copyright (c) Microsoft Corporation. All rights reserved.
   Licensed under the MIT License.

********************************************************************/

#include "winfile.h"
#include <shlobj.h>
#include <commctrl.h>

#ifdef PROGMAN
#include "wficon.h"
#endif

#define DO_DROPFILE 0x454C4946L
#define DO_PRINTFILE 0x544E5250L
#define DO_DROPONDESKTOP 0x504D42L

HWND hwndGlobalSink = NULL;

VOID  SelectItem(HWND hwndLB, WPARAM wParam, BOOL bSel);
VOID  ShowItemBitmaps(HWND hwndLB, BOOL bShow);

HCURSOR
GetMoveCopyCursor()
{
   if (fShowSourceBitmaps) {
      // copy
      return LoadCursor(hAppInstance, (LPTSTR) MAKEINTRESOURCE(iCurDrag | 1));
   } else {
      // move
      return LoadCursor(hAppInstance, (LPTSTR) MAKEINTRESOURCE(iCurDrag & ~1));
   }
}


/////////////////////////////////////////////////////////////////////
//
// Name:     MatchFile
//
// Synopsis: Match dos wildcard spec vs. dos filename
//           Both strings in uppercase
//
// Return:
//
//
// Assumes:
//
// Effects:
//
//
// Notes:
//
/////////////////////////////////////////////////////////////////////

BOOL
MatchFile(LPWSTR szFile, LPWSTR szSpec)
{

#define IS_DOTEND(ch)   ((ch) == CHAR_DOT || (ch) == CHAR_NULL)

   if (!lstrcmp(szSpec, SZ_STAR) ||            // "*" matches everything
      !lstrcmp(szSpec, szStarDotStar))         // so does "*.*"
      return TRUE;

   while (*szFile && *szSpec) {

      switch (*szSpec) {
      case CHAR_QUESTION:
         szFile++;
         szSpec++;
         break;

      case CHAR_STAR:

         while (!IS_DOTEND(*szSpec))     // got till a terminator
            szSpec = CharNext(szSpec);

         if (*szSpec == CHAR_DOT)
            szSpec++;

         while (!IS_DOTEND(*szFile))     // got till a terminator
            szFile = CharNext(szFile);

         if (*szFile == CHAR_DOT)
            szFile++;

         break;

      default:
         if (*szSpec == *szFile) {

            szFile++;
            szSpec++;
         } else
            return FALSE;
      }
   }
   return !*szFile && !*szSpec;
}


VOID
DSSetSelection(
   HWND hwndLB,
   BOOL bSelect,
   LPWSTR szSpec,
   BOOL bSearch)
{
   INT i;
   INT iMac;
   LPXDTA lpxdta;
   LPXDTALINK lpStart;
   WCHAR szTemp[MAXPATHLEN];

   CharUpper(szSpec);

   lpStart = (LPXDTALINK)GetWindowLong(GetParent(hwndLB), GWL_HDTA);

   if (!lpStart)
      return;

   iMac = (INT)MemLinkToHead(lpStart)->dwEntries;

   for (i = 0; i < iMac; i++) {

      if (SendMessage(hwndLB, LB_GETTEXT, i, (LPARAM)&lpxdta) == LB_ERR)
         return;

      if (!lpxdta || lpxdta->dwAttrs & ATTR_PARENT)
         continue;

      lstrcpy(szTemp, MemGetFileName(lpxdta));

      if (bSearch) {
         StripPath(szTemp);
      }

      CharUpper(szTemp);

      if (MatchFile(szTemp, szSpec))
         SendMessage(hwndLB, LB_SETSEL, bSelect, i);
   }
}


/*--------------------------------------------------------------------------*/
/*                                                                          */
/*  ShowItemBitmaps() -                                                     */
/*                                                                          */
/*--------------------------------------------------------------------------*/

VOID
ShowItemBitmaps(HWND hwndLB, BOOL bShow)
{
   INT i;
   INT iMac;
   INT iFirstSel;
   RECT rc;
   INT dx;
   LPINT lpSelItems;

   if (bShow == fShowSourceBitmaps)
      return;

   fShowSourceBitmaps = bShow;

#ifdef PROGMAN
   dx = ((GetWindowLong(GetParent(GetParent(hwndLB)),GWL_VIEW) & VIEW_ICON) ?
            dxIcon :
            dxFolder) +

        dyBorderx2 + dyBorder;
#else
   dx = dxFolder + dyBorderx2 + dyBorder;
#endif

   //
   // Invalidate the bitmap parts of all visible, selected items.
   //
   iFirstSel = (INT)SendMessage(hwndLB, LB_GETTOPINDEX, 0, 0L);
   iMac = (INT) SendMessage(hwndLB, LB_GETSELCOUNT, 0, 0L);

   if (iMac == LB_ERR)
      return;

   lpSelItems = (LPINT) LocalAlloc(LMEM_FIXED, sizeof(INT) * iMac);

   if (lpSelItems == NULL)
      return;

   iMac = (INT)SendMessage(hwndLB,
                           LB_GETSELITEMS,
                           (WPARAM)iMac,
                           (LPARAM)lpSelItems);

   for(i=0; i<iMac; i++) {

      if (lpSelItems[i] < iFirstSel)
         continue;

      if (SendMessage(hwndLB,
                      LB_GETITEMRECT,
                      lpSelItems[i],
                      (LPARAM)&rc) == LB_ERR)
         break;

      //
      // Invalidate the bitmap area.
      //
      rc.right = rc.left + dx;
      InvalidateRect(hwndLB, &rc, FALSE);
   }
   UpdateWindow(hwndLB);

   LocalFree((HLOCAL)lpSelItems);
}



/////////////////////////////////////////////////////////////////////
//
// Name:     SelectItem
//
// Synopsis:
//
// Return:
//
//
// Assumes:
//
// Effects:
//
//
// Notes:
//
/////////////////////////////////////////////////////////////////////

VOID
SelectItem(HWND hwndLB, WPARAM wParam, BOOL bSel)
{
   //
   // Add the current item to the selection.
   //
   SendMessage(hwndLB, LB_SETSEL, bSel, (DWORD)wParam);

   //
   // Give the selected item the focus rect and anchor pt.
   //
   SendMessage(hwndLB, LB_SETCARETINDEX, wParam, MAKELONG(TRUE,0));
   SendMessage(hwndLB, LB_SETANCHORINDEX, wParam, 0L);
}



/////////////////////////////////////////////////////////////////////
//
// Name:     DSDragLoop
//
// Synopsis: Called by dir and search drag loops.  Must handle
//           detecting all kinds of different destinations.
//
//      hwndLB  source listbox (either the dir or the sort)
//      wParam  same as sent for WM_DRAGLOOP (TRUE if on a dropable sink)
//      lpds    drop struct sent with the message
//      bSearch TRUE if we are in the search listbox
//
// Return:
//
//
// Assumes:
//
// Effects:
//
//
// Notes:
//
/////////////////////////////////////////////////////////////////////

VOID
DSDragLoop(HWND hwndLB, WPARAM wParam, LPDROPSTRUCT lpds)
{
   BOOL bShowBitmap;
   LPXDTA lpxdta;
   HWND hwndMDIChildSink, hwndDir;
   BOOL bForceMoveCur = FALSE;

   //
   // bShowBitmap is used to turn the source bitmaps on or off to distinguish
   // between a move and a copy or to indicate that a drop can
   // occur (exec and app)
   //
   // hack: keep around for drop files!
   //
   hwndGlobalSink = lpds->hwndSink;

   //
   // default to copy
   //
   bShowBitmap = TRUE;

   //
   // can't drop here
   //
   if (!wParam)
      goto DragLoopCont;

   //
   // Is the user holding down the CTRL key (which forces a copy)?
   //
   if (GetKeyState(VK_CONTROL) < 0) {
       bShowBitmap = TRUE;
       goto DragLoopCont;
   }

   //
   // Is the user holding down the ALT or SHIFT key (which forces a move)?
   //
   if (GetKeyState(VK_MENU)<0 || GetKeyState(VK_SHIFT)<0) {
      bShowBitmap = FALSE;
      goto DragLoopCont;
   }

   hwndMDIChildSink = GetMDIChildFromDecendant(lpds->hwndSink);

   //
   // Are we over the source listbox? (sink and source the same)
   //
   if (lpds->hwndSink == hwndLB) {

      //
      // Are we over a valid listbox entry?
      //
      if (lpds->dwControlData == (DWORD)-1) {

         //
         // Now force move cursor
         //
         bForceMoveCur = TRUE;
         goto DragLoopCont;

      } else {

         //
         // are we over a directory entry?
         //
         SendMessage(hwndLB, LB_GETTEXT, (WPARAM)(lpds->dwControlData), (LPARAM)&lpxdta);

         if (!(lpxdta && lpxdta->dwAttrs & ATTR_DIR)) {

            //
            // Now force move cursor
            //
            bForceMoveCur = TRUE;

            goto DragLoopCont;
         }
      }
   }

   //
   // Now we need to see if we are over an Executable file.  If so, we
   // need to force the Bitmaps to draw.
   //

   //
   // Are we over a directory window?
   //
   if (hwndMDIChildSink)
      hwndDir = HasDirWindow(hwndMDIChildSink);
   else
      hwndDir = NULL;

   if (hwndDir && (hwndDir == GetParent(lpds->hwndSink))) {

      //
      // Are we over an occupided part of the list box?
      //
      if (lpds->dwControlData != (DWORD)-1) {

         //
         // Are we over an Executable?
         //
         SendMessage(lpds->hwndSink, LB_GETTEXT, (WORD)(lpds->dwControlData), (LPARAM)(LPTSTR)&lpxdta);

         if (lpxdta && IsProgramFile(MemGetFileName(lpxdta))) {
            goto DragLoopCont;
         }
      }
   }

   //
   // Are we dropping into the same drive (check the source and dest drives)
   //
   bShowBitmap = ((INT)SendMessage(GetParent(hwndLB), FS_GETDRIVE, 0, 0L) !=
                  GetDrive(lpds->hwndSink, lpds->ptDrop));

DragLoopCont:

   ShowItemBitmaps(hwndLB, bShowBitmap);

   //
   // hack, set the cursor to match the move/copy state
   //
   if (wParam) {
      if (bForceMoveCur) {
         SetCursor(LoadCursor(hAppInstance, (LPTSTR) MAKEINTRESOURCE(iCurDrag & ~1)));
      } else {
         SetCursor(GetMoveCopyCursor());
      }
   }
}


/////////////////////////////////////////////////////////////////////
//
// Name:     DSRectItem()
//
// Synopsis: Rect the drop sink and update the status bar
//
// Return:
// Assumes:
// Effects:
// Notes:
//
/////////////////////////////////////////////////////////////////////

VOID
DSRectItem(
   HWND hwndLB,
   INT iItem,
   BOOL bFocusOn,
   BOOL bSearch)
{
   RECT      rc;
   RECT      rcT;
   HDC       hDC;
   BOOL      bSel;
   INT       nColor;
   HBRUSH    hBrush;
   LPXDTA    lpxdta;
   WCHAR     szTemp[MAXPATHLEN];
   PDOCBUCKET pIsProgram = NULL;
   LPWSTR    pszFile;

   //
   // Are we over an unused part of the listbox?
   //
   if (iItem == -1) {
      if (bSearch || hwndDragging == hwndLB) {
         SendMessage(hwndStatus,
                     SB_SETTEXT,
                     SBT_NOBORDERS|255,
                     (LPARAM)szNULL);

         UpdateWindow(hwndStatus);

      } else {

         SendMessage(GetParent(hwndLB),
                     FS_GETDIRECTORY,
                     COUNTOF(szTemp),
                     (LPARAM)szTemp);

         StripBackslash(szTemp);

         SetStatusText(SBT_NOBORDERS|255,
                       SST_RESOURCE|SST_FORMAT,
                       (LPWSTR)(fShowSourceBitmaps ?
                          IDS_DRAG_COPYING :
                          IDS_DRAG_MOVING),
                       szTemp);

         UpdateWindow(hwndStatus);
      }
      return;
   }

   //
   // Are we over ourselves? (i.e. a selected item in the source listbox)
   //
   bSel = (BOOL)SendMessage(hwndLB, LB_GETSEL, iItem, 0L);

   if (bSel && (hwndDragging == hwndLB)) {

ClearStatus:

      SendMessage(hwndStatus,
                  SB_SETTEXT,
                  SBT_NOBORDERS|255,
                  (LPARAM)szNULL);

      UpdateWindow(hwndStatus);
      return;
   }

   //
   // We only put rectangles around directories and program items.
   //
   if (SendMessage(hwndLB,
                   LB_GETTEXT,
                   iItem,
                   (LPARAM)(LPTSTR)&lpxdta) == LB_ERR || !lpxdta) {
      return;
   }

   if (!(lpxdta->dwAttrs & ATTR_DIR)  &&
      !(pIsProgram = IsProgramFile(MemGetFileName(lpxdta)))) {

      //
      // It's not a directory
      //

      //
      // If it's the same dir window, or we are dropping to a search
      // window, don't show any text!
      //
      if ((hwndDragging == hwndLB) || bSearch) {
         goto ClearStatus;
      }

      //
      // We are in a directory window (not search window)
      // but we aren't over a folder, so just use the curernt directory.
      //
      SendMessage(GetParent(hwndLB), FS_GETDIRECTORY, COUNTOF(szTemp), (LPARAM)szTemp);
      StripBackslash(szTemp);

      SetStatusText(SBT_NOBORDERS|255,
                    SST_FORMAT | SST_RESOURCE,
                    (LPWSTR)(fShowSourceBitmaps ?
                        IDS_DRAG_COPYING :
                        IDS_DRAG_MOVING),
                    szTemp);

      UpdateWindow(hwndStatus);
      return;
   }

   //
   // At this point, we are over a directory folder,
   // be we could be in a search or directory window.
   //

   //
   // Turn the item's rectangle on or off.
   //
   if (bSearch || !(lpxdta->dwAttrs & ATTR_PARENT)) {

      pszFile = MemGetFileName(lpxdta);

   } else {

      SendMessage(GetParent(hwndLB), FS_GETDIRECTORY, COUNTOF(szTemp), (LPARAM)szTemp);
      StripBackslash(szTemp);  // trim it down
      StripFilespec(szTemp);

      pszFile = szTemp;
   }

   //
   // Else bSearch and szTemp contains the file name
   //
   if (bFocusOn) {

      SetStatusText(SBT_NOBORDERS|255,
                    SST_FORMAT | SST_RESOURCE,
                    (LPWSTR)(pIsProgram ?
                       IDS_DRAG_EXECUTING :
                       (fShowSourceBitmaps ?
                          IDS_DRAG_COPYING :
                          IDS_DRAG_MOVING)),
                   pszFile);

      UpdateWindow(hwndStatus);
   }

   SendMessage(hwndLB, LB_GETITEMRECT, iItem, (LPARAM)(LPRECT)&rc);
   GetClientRect(hwndLB,&rcT);
   IntersectRect(&rc,&rc,&rcT);

   if (bFocusOn) {
      hDC = GetDC(hwndLB);
      if (bSel) {
         nColor = COLOR_WINDOW;
         InflateRect(&rc, -1, -1);
      } else
         nColor = COLOR_WINDOWFRAME;

      if (hBrush = CreateSolidBrush(GetSysColor(nColor))) {
         FrameRect(hDC, &rc, hBrush);
         DeleteObject(hBrush);
      }
      ReleaseDC(hwndLB, hDC);
   } else {
      InvalidateRect(hwndLB, &rc, FALSE);
      UpdateWindow(hwndLB);
   }
}


/*--------------------------------------------------------------------------*/
/*                                                                          */
/*  DropFilesOnApplication() -                                              */
/*                                                                          */
/*--------------------------------------------------------------------------*/

/* this function will determine whether the application we are currently
 * over is a valid drop point and drop the files
 */

WORD
DropFilesOnApplication(LPTSTR pszFiles)
{
    POINT pt;
    HWND hwnd;
    RECT rc;
    HANDLE hDrop,hT;
    LPBYTE lpList;
    UINT cbList;
    UINT cbT;

    LPDROPFILES lpdfs;
    TCHAR szFile[MAXPATHLEN];

    if (!(hwnd = hwndGlobalSink))
        return 0;

    hwndGlobalSink = NULL;

    GetCursorPos(&pt);

    cbList = 2*sizeof(*szFile) + sizeof(DROPFILES);

    hDrop = GlobalAlloc(GMEM_DDESHARE|GMEM_MOVEABLE|GMEM_ZEROINIT, cbList);
    if (!hDrop)
        return 0;

    lpdfs = (LPDROPFILES)GlobalLock(hDrop);

    GetClientRect(hwnd,&rc);
    ScreenToClient(hwnd,&pt);
    lpdfs->pt = pt;
    lpdfs->fNC = !PtInRect(&rc,pt);
    lpdfs->pFiles = sizeof(DROPFILES);

    lpdfs->fWide = TRUE;

    GlobalUnlock(hDrop);

    while (pszFiles = GetNextFile(pszFiles, szFile, COUNTOF(szFile))) {

       QualifyPath(szFile);

       cbT = ByteCountOf(lstrlen(szFile)+1);

       hT = GlobalReAlloc(hDrop, cbList + cbT, GMEM_MOVEABLE|GMEM_ZEROINIT);

       if (!hT)
          break;

       hDrop = hT;
       lpList = GlobalLock(hDrop);

       lstrcpy((LPTSTR)(lpList+cbList-2*sizeof(*szFile)), szFile);

       GlobalUnlock(hDrop);
       cbList += cbT;
    }

    PostMessage(hwnd, WM_DROPFILES, (WPARAM)hDrop, 0L);

    return 1;
}


/////////////////////////////////////////////////////////////////////
//
// Name:     DSTrackPoint
//
// Synopsis:
//
// Return:   0 for normal mouse tracking
//           1 for no mouse single click processing
//           2 for no mouse single- or double-click tracking
//
//
// Assumes:
//
// Effects:
//
//
// Notes:
//
/////////////////////////////////////////////////////////////////////

INT
DSTrackPoint(
   HWND hwnd,
   HWND hwndLB,
   WPARAM wParam,
   LPARAM lParam,
   BOOL bSearch)
{
   UINT      iSel;
   MSG       msg;
   RECT      rc;
   DWORD     dwAnchor;
   DWORD     dwTemp;
   LPWSTR    pch;
   BOOL      bDir;
   BOOL      bSelected;
   BOOL      bSelectOneItem;
   BOOL      bUnselectIfNoDrag;
   LPWSTR    pszFile;
   POINT     pt;

   bSelectOneItem = FALSE;
   bUnselectIfNoDrag = FALSE;

   bSelected = (BOOL)SendMessage(hwndLB, LB_GETSEL, wParam, 0L);

   if (GetKeyState(VK_SHIFT) < 0) {

      // What is the state of the Anchor point?
      dwAnchor = SendMessage(hwndLB, LB_GETANCHORINDEX, 0, 0L);
      bSelected = (BOOL)SendMessage(hwndLB, LB_GETSEL, dwAnchor, 0L);

      // If Control is up, turn everything off.

      if (!(GetKeyState(VK_CONTROL) < 0))
         SendMessage(hwndLB, LB_SETSEL, FALSE, -1L);

      // Select everything between the Anchor point and the item.
      SendMessage(hwndLB, LB_SELITEMRANGE, bSelected, MAKELONG(wParam, dwAnchor));

      // Give the selected item the focus rect.
      SendMessage(hwndLB, LB_SETCARETINDEX, wParam, 0L);

   } else if (GetKeyState(VK_CONTROL) < 0) {
      if (bSelected)
         bUnselectIfNoDrag = TRUE;
      else
         SelectItem(hwndLB, wParam, TRUE);

   } else {
      if (bSelected)
         bSelectOneItem = TRUE;
      else {
         // Deselect everything.
         SendMessage(hwndLB, LB_SETSEL, FALSE, -1L);

         // Select the current item.
         SelectItem(hwndLB, wParam, TRUE);
      }
   }

   if (!bSearch)
      UpdateStatus(GetParent(hwnd));

   POINTSTOPOINT(pt, lParam);
   ClientToScreen(hwndLB, (LPPOINT)&pt);
   ScreenToClient(hwnd, (LPPOINT)&pt);

   // See if the user moves a certain number of pixels in any direction

   SetRect(&rc, pt.x - dxClickRect, pt.y - dyClickRect,
      pt.x + dxClickRect, pt.y + dyClickRect);

   SetCapture(hwnd);

   for (;;) {

      if (GetCapture() != hwnd) {
          msg.message = WM_LBUTTONUP;   // don't proceed below
          break;
      }

      if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
          DispatchMessage(&msg);

          //
          // WM_CANCELMODE messages will unset the capture, in that
          // case I want to exit this loop

          //
          // Must do explicit check.
          //
      if (msg.message == WM_CANCELMODE || GetCapture() != hwnd) {
              msg.message = WM_LBUTTONUP;   // don't proceed below

              break;
          }

          if (msg.message == WM_LBUTTONUP)
              break;

          POINTSTOPOINT(pt, msg.lParam);
          if ((msg.message == WM_MOUSEMOVE) && !(PtInRect(&rc, pt)))
              break;
      }
    }
    ReleaseCapture();

    // Did the guy NOT drag anything?
    if (msg.message == WM_LBUTTONUP) {
       if (bSelectOneItem) {
          /* Deselect everything. */
          SendMessage(hwndLB, LB_SETSEL, FALSE, -1L);

          /* Select the current item. */
          SelectItem(hwndLB, wParam, TRUE);
       }

      if (bUnselectIfNoDrag)
         SelectItem(hwndLB, wParam, FALSE);

      // notify the appropriate people

      SendMessage(hwnd, WM_COMMAND,
         GET_WM_COMMAND_MPS(0, hwndLB, LBN_SELCHANGE));

      return 1;
   }

   //
   // Enter Danger Mouse's BatCave.
   //
   if (SendMessage(hwndLB, LB_GETSELCOUNT, 0, 0L) == 1) {

      LPXDTA lpxdta;

      //
      // There is only one thing selected.
      //  Figure out which cursor to use.
      //
      // There is only one thing selected.
      //  Figure out which cursor to use.

      if (SendMessage(hwndLB,
                      LB_GETTEXT,
                      wParam,
                      (LPARAM)&lpxdta) == LB_ERR || !lpxdta) {
         return 1;
      }

      pszFile = MemGetFileName(lpxdta);

      bDir = lpxdta->dwAttrs & ATTR_DIR;

      //
      // avoid dragging the parrent dir
      //
      if (lpxdta->dwAttrs & ATTR_PARENT) {
         return 1;
      }

      if (bDir) {
         iSel = DOF_DIRECTORY;
      } else if (IsProgramFile(pszFile)) {
         iSel = DOF_EXECUTABLE;
      } else if (IsDocument(pszFile)) {
         iSel = DOF_DOCUMENT;
      } else
         iSel = DOF_DOCUMENT;

      iCurDrag = SINGLECOPYCURSOR;
   } else {

      // Multiple files are selected.
      iSel = DOF_MULTIPLE;
      iCurDrag = MULTCOPYCURSOR;
   }


   // Get the list of selected things.
   pch = (LPTSTR)SendMessage(hwnd, FS_GETSELECTION, FALSE, 0L);

   // Wiggle things around.
   hwndDragging = hwndLB;

   dwTemp = DragObject(GetDesktopWindow(), hwnd, (UINT)iSel, (DWORD)pch, NULL);

   SetWindowDirectory();

   if (dwTemp == DO_PRINTFILE) {
      // print these suckers
      hdlgProgress = NULL;
      WFPrint(pch);
   } else if (dwTemp == DO_DROPFILE) {
      // try and drop them on an application
      DropFilesOnApplication(pch);
   }

   LocalFree((HANDLE)pch);

   if (IsWindow(hwnd))
      ShowItemBitmaps(hwndLB, TRUE);

   hwndDragging = NULL;

   if (!bSearch && IsWindow(hwnd))
      UpdateStatus(GetParent(hwnd));

   return 2;
}


/////////////////////////////////////////////////////////////////////
//
// Name:     SkipPathHead
//
// Synopsis: Skips "C:\" and "\\foo\bar\"
//
// INC       lpszPath -- path to check
//
// Return:   LPTSTR   pointer to first filespec
//
// Assumes:
//
// Effects:
//
//
// Notes:    If not fully qualified, returns NULL
//
/////////////////////////////////////////////////////////////////////

LPTSTR
SkipPathHead(LPTSTR lpszPath)
{
   LPTSTR p = lpszPath;
   INT i;

   if (ISUNCPATH(p)) {

      for(i=0, p+=2; *p && i<2; p++) {

         if (CHAR_BACKSLASH == *p)
            i++;
      }

      //
      // If we ran out of string, punt.
      //
      if (!*p)
         return NULL;
      else
         return p;

   } else if (CHAR_COLON == lpszPath[1] && CHAR_BACKSLASH == lpszPath[2]) {

      //
      // Regular pathname
      //

      return lpszPath+3;
   }

   return NULL;
}


BOOL
DSDropObject(
   HWND hwndHolder,
   HWND hwndLB,
   LPDROPSTRUCT lpds,
   BOOL bSearch)
{
   DWORD      ret;
   LPWSTR     pFrom;
   DWORD      dwAttrib = 0;       // init this to not a dir
   DWORD      dwSelSink;
   LPWSTR     pSel;
   LPWSTR     pSelNoQuote;
   LPXDTA     lpxdta;
   LPXDTALINK lpStart;

   WCHAR szTemp[MAXPATHLEN*2];
   WCHAR szSourceFile[MAXPATHLEN+2];
   WCHAR szSourceFileQualified[MAXPATHLEN+2];

   //
   // Turn off status bar
   //
   SendMessage(hwndStatus, SB_SIMPLE, 0, 0L);
   UpdateWindow(hwndStatus);

   //
   // this is the listbox index of the destination
   //
   dwSelSink = lpds->dwControlData;

   //
   // Are we dropping onto ourselves? (i.e. a selected item in the
   // source listbox OR an unused area of the source listbox)  If
   // so, don't do anything.
   //
   if (hwndHolder == lpds->hwndSource) {
      if ((dwSelSink == (DWORD)-1) || SendMessage(hwndLB, LB_GETSEL, dwSelSink, 0L))
         return TRUE;
   }

   //
   // set the destination, assume move/copy case below (c:\foo\)
   //
   SendMessage(hwndHolder, FS_GETDIRECTORY, COUNTOF(szTemp), (LPARAM)szTemp);

   //
   // Are we dropping on a unused portion of some listbox?
   //
   if (dwSelSink == (DWORD)-1) {
      goto NormalMoveCopy;
   }

   //
   // check for drop on a directory
   //
   lpStart = (LPXDTALINK)GetWindowLong(hwndHolder, GWL_HDTA);

   //
   // If dropping on "No files." or "Access denied." then do normal thing.
   //
   if (!lpStart)
      goto NormalMoveCopy;

   if (SendMessage(hwndLB,
                   LB_GETTEXT,
                   dwSelSink,
                   (LPARAM)&lpxdta) == LB_ERR || !lpxdta) {
      goto NormalMoveCopy;
   }

   lstrcpy(szSourceFile, MemGetFileName(lpxdta));
   dwAttrib = lpxdta->dwAttrs;

   if (dwAttrib & ATTR_DIR) {

      if (bSearch) {

         lstrcpy(szTemp, szSourceFile);

      } else {

         //
         // special case the parent
         //
         if (dwAttrib & ATTR_PARENT) {
            StripBackslash(szTemp);
            StripFilespec(szTemp);
         } else {
            lstrcat(szTemp, szSourceFile);
         }
      }
      goto DirMoveCopy;
   }

   //
   // dropping on a program?
   //
   if (!IsProgramFile(szSourceFile))
      goto NormalMoveCopy;

   //
   // directory drop on a file? this is a NOP
   //
   if (lpds->wFmt == DOF_DIRECTORY) {
      DSRectItem(hwndLB, iSelHilite, FALSE, FALSE);
      return FALSE;
   }

   //
   // We're dropping a file onto a program.
   // Exec the program using the source file as the parameter.
   //
   // set the directory to that of the program to exec
   //
   SendMessage(hwndHolder, FS_GETDIRECTORY, COUNTOF(szTemp), (LPARAM)szTemp);
   StripBackslash(szTemp);

   SetCurrentDirectory(szTemp);

   //
   // We need a fully qualified version of the exe since SheConvertPath
   // doesn't check the current directory if it finds the exe in the path.
   //
   lstrcpy(szSourceFileQualified, szTemp);
   lstrcat(szSourceFileQualified, SZ_BACKSLASH);
   lstrcat(szSourceFileQualified, szSourceFile);

   //
   // get the selected file
   //
   pSel = (LPWSTR)SendMessage(lpds->hwndSource, FS_GETSELECTION, 1, 0L);
   pSelNoQuote = (LPWSTR)SendMessage(lpds->hwndSource, FS_GETSELECTION, 1|16, 0L);
   if (!pSel || !pSelNoQuote)
   {
      goto DODone;
   }

   if (lstrlen(pSel) > MAXPATHLEN)   // don't blow up below!
      goto DODone;

   //
   // Checkesc on target exe
   //
   CheckEsc(szSourceFile);

   if (bConfirmMouse) {

      LoadString(hAppInstance, IDS_MOUSECONFIRM, szTitle, COUNTOF(szTitle));
      LoadString(hAppInstance, IDS_EXECMOUSECONFIRM, szTemp, COUNTOF(szTemp));

      wsprintf(szMessage, szTemp, szSourceFile, pSel);
      if (MessageBox(hwndFrame, szMessage, szTitle, MB_YESNO | MB_ICONEXCLAMATION) != IDYES)
         goto DODone;
   }


   //
   // create an absolute path to the argument (search window alaready
   // is absolute)
   //
   if (lpds->hwndSource == hwndSearch) {
      szTemp[0] = CHAR_NULL;
   } else {
      SendMessage(lpds->hwndSource, FS_GETDIRECTORY, COUNTOF(szTemp), (LPARAM)szTemp);
   }

   lstrcat(szTemp, pSelNoQuote);     // this is the parameter to the exec

   //
   // put a "." extension on if none found
   //
   if (*GetExtension(szTemp) == 0)
      lstrcat(szTemp, SZ_DOT);

   //
   // Since it's only 1 filename, checkesc it
   //
   CheckEsc(szTemp);

   ret = ExecProgram(szSourceFileQualified, szTemp, NULL, FALSE);

   if (ret)
      MyMessageBox(hwndFrame, IDS_EXECERRTITLE, (WORD)ret, MB_OK | MB_ICONEXCLAMATION | MB_SYSTEMMODAL);

DODone:
   DSRectItem(hwndLB, iSelHilite, FALSE, FALSE);
   if (pSel)
   {
      LocalFree((HANDLE)pSel);
   }
   if (pSelNoQuote)
   {
      LocalFree((HANDLE)pSelNoQuote);
   }
   return TRUE;

   // szTemp must not be checkesc'd here.

NormalMoveCopy:
   //
   // Make sure that we don't move into same dir.
   //
   if (GetWindowLong(hwndHolder,
                     GWL_LISTPARMS) == SendMessage(hwndMDIClient,
                                                   WM_MDIGETACTIVE,
                                                   0,
                                                   0L)) {
      return TRUE;
   }

DirMoveCopy:

   //
   // the source filename is in the loword
   //
   pFrom = (LPWSTR)lpds->dwData;

   AddBackslash(szTemp);
   lstrcat(szTemp, szStarDotStar);   // put files in this dir

   //
   // again moved here: target is single!
   //
   CheckEsc(szTemp);

   ret = DMMoveCopyHelper(pFrom, szTemp, fShowSourceBitmaps);

   DSRectItem(hwndLB, iSelHilite, FALSE, FALSE);

   if (ret)
      return TRUE;

#if 0
   if (!fShowSourceBitmaps)
      SendMessage(lpds->hwndSource, WM_FSC, FSC_REFRESH, 0L);

   //
   // we got dropped on, but if this is a dir we don't need to refresh
   //
   if (!(dwAttrib & ATTR_DIR))
      SendMessage(hwndHolder, WM_FSC, FSC_REFRESH, 0L);
#endif

   return TRUE;
}
