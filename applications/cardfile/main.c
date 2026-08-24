/*!

   (c) osFree Project 2002-2026, <https://www.osFree.org>
 
   SPDX-License-Identifier: BSD-3-Clause

*/

/* main.c */
#include <windows.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "cardfile.h"

#define JUMP 8   /* шаг для кнопок << и >> */

/* Глобальные переменные */
HINSTANCE hInst;
HWND      hMainWnd;
HWND      hListBox, hEdit;
HWND      hFindDlg = NULL;
BOOL      isModified = FALSE;
char      szFileName[128] = "";
char      szFileTitle[64] = "Untitled";
char      szFindStr[128] = "";

/* Прототипы */
LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);
BOOL CALLBACK    FileOpenDlgProc(HWND, UINT, WPARAM, LPARAM);
BOOL CALLBACK    SearchDlgProc(HWND, UINT, WPARAM, LPARAM);
BOOL CALLBACK    AddDlgProc(HWND, UINT, WPARAM, LPARAM);
BOOL CALLBACK    IndexDlgProc(HWND, UINT, WPARAM, LPARAM);
BOOL CALLBACK    AboutDlgProc(HWND, UINT, WPARAM, LPARAM);
void             RefreshListBox(void);
void             SaveCurrentCard(void);
void             LoadCardToEdit(CardNode *node);
void             UpdateTitle(void);
BOOL             AskSave(void);
BOOL             PerformSave(void);
BOOL             PerformSaveAs(void);
void             GotoCard(int offset);
void             GotoCardByLetter(char c);    /* <-- добавлен прототип */
void             MergeCards(const char *filename);

/*--------------------------------------------------------------------*/
int PASCAL WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance,
                   LPSTR lpCmdLine, int nCmdShow)
{
    WNDCLASS wc;
    HWND     hwnd;
    MSG      msg;
    HACCEL   hAccel;
    hInst = hInstance;

    if (!hPrevInstance) {
        wc.style         = CS_HREDRAW | CS_VREDRAW;
        wc.lpfnWndProc   = WndProc;
        wc.cbClsExtra    = 0;
        wc.cbWndExtra    = 0;
        wc.hInstance     = hInstance;
        wc.hIcon         = LoadIcon(NULL, IDI_APPLICATION);
        wc.hCursor       = LoadCursor(NULL, IDC_ARROW);
        wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
        wc.lpszMenuName  = "MAINMENU";
        wc.lpszClassName = "WincardsClass";
        if (!RegisterClass(&wc)) return 0;
    }

    hwnd = CreateWindow("WincardsClass", "Wincards",
                        WS_OVERLAPPEDWINDOW,
                        CW_USEDEFAULT, CW_USEDEFAULT,
                        640, 450,
                        NULL, NULL, hInstance, NULL);
    if (!hwnd) return 0;

    ShowWindow(hwnd, nCmdShow);
    UpdateWindow(hwnd);

    hAccel = LoadAccelerators(hInst, "ACCELTABLE");

    while (GetMessage(&msg, NULL, 0, 0)) {
        if (hFindDlg && IsDialogMessage(hFindDlg, &msg))
            continue;
        if (!TranslateAccelerator(hwnd, hAccel, &msg)) {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }
    return msg.wParam;
}

/*--------------------------------------------------------------------*/
LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    switch (msg) {
    case WM_CREATE: {
        RECT rc;
        GetClientRect(hwnd, &rc);
        /* кнопки навигации */
        CreateWindow("BUTTON", "<<", WS_CHILD|WS_VISIBLE|BS_PUSHBUTTON,
                     0,0,40,20, hwnd, (HMENU)BTN_FIRST, hInst, NULL);
        CreateWindow("BUTTON", "<", WS_CHILD|WS_VISIBLE|BS_PUSHBUTTON,
                     40,0,40,20, hwnd, (HMENU)BTN_PREV, hInst, NULL);
        CreateWindow("BUTTON", ">", WS_CHILD|WS_VISIBLE|BS_PUSHBUTTON,
                     80,0,40,20, hwnd, (HMENU)BTN_NEXT, hInst, NULL);
        CreateWindow("BUTTON", ">>", WS_CHILD|WS_VISIBLE|BS_PUSHBUTTON,
                     120,0,40,20, hwnd, (HMENU)BTN_LAST, hInst, NULL);
        /* список индексов */
        hListBox = CreateWindow("LISTBOX", NULL,
                                WS_CHILD|WS_VISIBLE|WS_VSCROLL|LBS_NOTIFY,
                                0,20, rc.right, (rc.bottom-20)/2,
                                hwnd, (HMENU)IDC_LISTBOX, hInst, NULL);
        /* редактор текста */
        hEdit = CreateWindow("EDIT", NULL,
                             WS_CHILD|WS_VISIBLE|WS_VSCROLL|
                             ES_MULTILINE|ES_AUTOVSCROLL,
                             0,20+(rc.bottom-20)/2, rc.right, (rc.bottom-20)/2,
                             hwnd, (HMENU)IDC_EDIT, hInst, NULL);
        hMainWnd = hwnd;
        /* начальная пустая карточка */
        {
            CardNode *node;
            CardNode *p;
            node = CreateCardNode("", "");
            if (node) {
                p = node;
                InsertCardSorted(&p);
                topcard = firstcard;
            }
        }
        RefreshListBox();
        return 0;
    }

    case WM_SIZE: {
        int w = LOWORD(lParam), h = HIWORD(lParam);
        MoveWindow(GetDlgItem(hwnd, BTN_FIRST), 0,0,40,20, TRUE);
        MoveWindow(GetDlgItem(hwnd, BTN_PREV), 40,0,40,20, TRUE);
        MoveWindow(GetDlgItem(hwnd, BTN_NEXT), 80,0,40,20, TRUE);
        MoveWindow(GetDlgItem(hwnd, BTN_LAST), 120,0,40,20, TRUE);
        MoveWindow(hListBox, 0,20, w, (h-20)/2, TRUE);
        MoveWindow(hEdit, 0,20+(h-20)/2, w, (h-20)/2, TRUE);
        return 0;
    }

    case WM_COMMAND:
        switch (LOWORD(wParam)) {
        case IDM_NEW:
            if (AskSave()) {
                DeleteAllCards();
                szFileName[0] = '\0';
                lstrcpy(szFileTitle, "Untitled");
                isModified = FALSE;
                {
                    CardNode *node;
                    CardNode *p;
                    node = CreateCardNode("", "");
                    if (node) {
                        p = node;
                        InsertCardSorted(&p);
                        topcard = firstcard;
                    }
                }
                UpdateTitle();
                RefreshListBox();
                SetWindowText(hEdit, "");
            }
            break;

        case IDM_OPEN:
            if (AskSave()) {
                DialogBox(hInst, "FILEOPEN", hwnd, FileOpenDlgProc);
            }
            break;

        case IDM_SAVE:
            if (szFileName[0]) PerformSave(); else PerformSaveAs();
            break;
        case IDM_SAVEAS:
            PerformSaveAs();
            break;
        case IDM_MERGE:
            DialogBox(hInst, "FILEOPEN", hwnd, FileOpenDlgProc);
            break;
        case IDM_EXIT:
            SendMessage(hwnd, WM_CLOSE, 0, 0);
            break;

        case IDM_COPY:
            SendMessage(hEdit, WM_COPY, 0, 0); break;
        case IDM_CUT:
            SendMessage(hEdit, WM_CUT, 0, 0); isModified = TRUE; break;
        case IDM_PASTE:
            SendMessage(hEdit, WM_PASTE, 0, 0); isModified = TRUE; break;
        case IDM_INDEX:
            if (topcard) DialogBox(hInst, "INDEXDLG", hwnd, IndexDlgProc);
            break;

        case IDM_ADD:
            SaveCurrentCard();
            if (DialogBox(hInst, "ADDDLG", hwnd, AddDlgProc))
                isModified = TRUE;
            break;
        case IDM_DELETE:
            if (topcard) {
                CardNode *nextcard = topcard->next;
                if (nextcard == topcard) nextcard = NULL;
                RemoveCardNode(topcard);
                FreeCardNode(topcard);
                topcard = nextcard;
                if (!firstcard) {
                    CardNode *node;
                    CardNode *p;
                    node = CreateCardNode("", "");
                    if (node) {
                        p = node;
                        InsertCardSorted(&p);
                        topcard = firstcard;
                    }
                }
                isModified = TRUE;
                RefreshListBox();
                LoadCardToEdit(topcard);
            }
            break;
        case IDM_DUP:
            if (topcard) {
                SaveCurrentCard();
                {
                    CardNode *node;
                    CardNode *p;
                    node = CreateCardNode(topcard->data.index, topcard->data.text);
                    if (node) {
                        p = node;
                        InsertCardSorted(&p);
                        topcard = node;
                        isModified = TRUE;
                        RefreshListBox();
                        LoadCardToEdit(topcard);
                    }
                }
            }
            break;

        case IDM_FIND:
            if (!hFindDlg)
                hFindDlg = CreateDialog(hInst, "FINDDLG", hwnd, SearchDlgProc);
            ShowWindow(hFindDlg, SW_SHOW);
            break;

        case IDM_ABOUT:
            DialogBox(hInst, "ABOUTDLG", hwnd, AboutDlgProc);
            break;

        case BTN_FIRST: GotoCard(-10000); break;
        case BTN_PREV:  GotoCard(-1); break;
        case BTN_NEXT:  GotoCard(1); break;
        case BTN_LAST:  GotoCard(10000); break;

        case SRCH_C: GotoCardByLetter('c'); break;
        case SRCH_V: GotoCardByLetter('v'); break;
        case SRCH_X: GotoCardByLetter('x'); break;

        case IDC_LISTBOX:
            if (HIWORD(wParam) == LBN_SELCHANGE) {
                int idx = SendMessage(hListBox, LB_GETCURSEL, 0, 0);
                if (idx >= 0) {
                    char buf[MAX_INDEX_LEN+1];
                    SendMessage(hListBox, LB_GETTEXT, idx, (LPARAM)buf);
                    SaveCurrentCard();
                    topcard = FindCardByIndex(buf);
                    if (topcard) LoadCardToEdit(topcard);
                }
            }
            break;
        }
        break;

    case WM_CLOSE:
        if (AskSave()) {
            if (hFindDlg) DestroyWindow(hFindDlg);
            DeleteAllCards();
            DestroyWindow(hwnd);
        }
        return 0;

    case WM_DESTROY:
        PostQuitMessage(0);
        return 0;
    }
    return DefWindowProc(hwnd, msg, wParam, lParam);
}

/* ========== Вспомогательные функции ========== */
void GotoCard(int offset)
{
    if (!topcard) return;
    SaveCurrentCard();
    if (offset == -10000) {
        topcard = firstcard;
    } else if (offset == 10000) {
        topcard = firstcard->prev;
    } else {
        MoveTopCard(offset);
    }
    LoadCardToEdit(topcard);
}

void GotoCardByLetter(char c)
{
    if (!firstcard) return;
    {
        char upper[2];
        CardNode *cur;
        upper[0] = (char)toupper((unsigned char)c);
        upper[1] = '\0';
        cur = topcard;
        do {
            if (cur->data.index[0] && toupper((unsigned char)cur->data.index[0]) == upper[0]) {
                SaveCurrentCard();
                topcard = cur;
                LoadCardToEdit(topcard);
                return;
            }
            cur = cur->next;
        } while (cur != topcard);
    }
}

void SaveCurrentCard(void)
{
    if (!topcard) return;
    {
        int len = GetWindowTextLength(hEdit);
        if (topcard->data.text) free(topcard->data.text);
        if (len > 0) {
            topcard->data.text = (char*) malloc(len + 1);
            if (topcard->data.text) {
                GetWindowText(hEdit, topcard->data.text, len + 1);
                topcard->data.textlen = len;
            } else topcard->data.textlen = 0;
        } else {
            topcard->data.text = NULL;
            topcard->data.textlen = 0;
        }
    }
}

void LoadCardToEdit(CardNode *node)
{
    if (!node) return;
    SetWindowText(hEdit, node->data.text ? node->data.text : "");
    {
        int idx = SendMessage(hListBox, LB_FINDSTRINGEXACT, -1, (LPARAM)node->data.index);
        if (idx >= 0) SendMessage(hListBox, LB_SETCURSEL, idx, 0);
    }
}

void RefreshListBox(void)
{
    CardNode *cur;
    SendMessage(hListBox, LB_RESETCONTENT, 0, 0);
    if (!firstcard) return;
    cur = firstcard;
    do {
        SendMessage(hListBox, LB_ADDSTRING, 0, (LPARAM)cur->data.index);
        cur = cur->next;
    } while (cur != firstcard);
    if (topcard) {
        int idx = SendMessage(hListBox, LB_FINDSTRINGEXACT, -1, (LPARAM)topcard->data.index);
        if (idx >= 0) SendMessage(hListBox, LB_SETCURSEL, idx, 0);
    }
}

void UpdateTitle(void)
{
    char title[256];
    wsprintf(title, "Wincards - %s%s", szFileTitle, isModified ? " (modified)" : "");
    SetWindowText(hMainWnd, title);
}

BOOL AskSave(void)
{
    if (isModified) {
        int res = MessageBox(hMainWnd, "Save changes?", "Wincards", MB_YESNOCANCEL|MB_ICONQUESTION);
        if (res == IDYES) {
            if (szFileName[0]) return PerformSave();
            else return PerformSaveAs();
        } else if (res == IDCANCEL) return FALSE;
    }
    return TRUE;
}

BOOL PerformSave(void)
{
    SaveCurrentCard();
    if (SaveCards(szFileName)) { isModified = FALSE; UpdateTitle(); return TRUE; }
    else { MessageBox(hMainWnd, "Failed to save.", "Error", MB_ICONERROR); return FALSE; }
}

BOOL PerformSaveAs(void)
{
    DialogBox(hInst, "FILESAVE", hMainWnd, FileOpenDlgProc);
    return TRUE;
}

void MergeCards(const char *filename)
{
    CardNode *oldfirst, *oldlast, *oldtop;
    oldfirst = firstcard; oldlast = lastcard; oldtop = topcard;
    firstcard = lastcard = topcard = NULL;
    if (LoadCards(filename)) {
        CardNode *cur = firstcard;
        if (cur) {
            do {
                CardNode *next = cur->next;
                CardNode *p;
                cur->prev->next = cur->next;
                cur->next->prev = cur->prev;
                if (cur == firstcard) firstcard = (cur->next == cur) ? NULL : cur->next;
                cur->prev = cur->next = NULL;
                p = cur;
                firstcard = oldfirst; lastcard = oldlast; topcard = oldtop;
                InsertCardSorted(&p);
                oldfirst = firstcard; oldlast = lastcard; oldtop = topcard;
                cur = next;
            } while (cur != firstcard);
        }
    }
    firstcard = oldfirst; lastcard = oldlast; topcard = oldtop;
    RefreshListBox();
}

/* ========== Диалоги ========== */
BOOL CALLBACK FileOpenDlgProc(HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
    static char filename[128] = "";
    switch (msg) {
    case WM_INITDIALOG:
        SetDlgItemText(hDlg, 500, "*.crd");
        return TRUE;
    case WM_COMMAND:
        if (LOWORD(wParam) == IDOK) {
            GetDlgItemText(hDlg, 500, filename, sizeof(filename));
            if (filename[0]) {
                lstrcpy(szFileName, filename);
                lstrcpy(szFileTitle, filename);
                {
                    char *p = strrchr(szFileTitle, '\\');
                    if (p) lstrcpy(szFileTitle, p+1);
                }
                DeleteAllCards();
                if (LoadCards(szFileName)) {
                    isModified = FALSE;
                    topcard = firstcard;
                    UpdateTitle();
                    RefreshListBox();
                    if (topcard) LoadCardToEdit(topcard);
                } else {
                    MessageBox(hDlg, "Failed to load file.", "Error", MB_ICONERROR);
                }
            }
            EndDialog(hDlg, TRUE);
            return TRUE;
        }
        if (LOWORD(wParam) == IDCANCEL) {
            EndDialog(hDlg, FALSE);
            return TRUE;
        }
        break;
    }
    return FALSE;
}

BOOL CALLBACK SearchDlgProc(HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
    static char buf[128];
    switch (msg) {
    case WM_INITDIALOG:
        SetDlgItemText(hDlg, IDD_SEARCH, szFindStr);
        return TRUE;
    case WM_COMMAND:
        if (LOWORD(wParam) == IDOK) {
            GetDlgItemText(hDlg, IDD_SEARCH, buf, sizeof(buf));
            if (buf[0]) {
                CardNode *found;
                lstrcpy(szFindStr, buf);
                found = FindCardByText(topcard ? topcard->next : firstcard, szFindStr);
                if (!found) found = FindCardByText(firstcard, szFindStr);
                if (found) {
                    SaveCurrentCard();
                    topcard = found;
                    LoadCardToEdit(topcard);
                    if (topcard->data.text) {
                        char *p = strstr(topcard->data.text, szFindStr);
                        if (p) {
                            int start = (int)(p - topcard->data.text);
                            SendMessage(hEdit, EM_SETSEL, start, start + lstrlen(szFindStr));
                        }
                    }
                } else MessageBeep(0);
            }
            EndDialog(hDlg, TRUE);
            return TRUE;
        }
        if (LOWORD(wParam) == IDCANCEL) { EndDialog(hDlg, FALSE); return TRUE; }
        break;
    }
    return FALSE;
}

BOOL CALLBACK AddDlgProc(HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
    static char buf[MAX_INDEX_LEN+1];
    switch (msg) {
    case WM_INITDIALOG:
        SendDlgItemMessage(hDlg, IDD_ADD, EM_LIMITTEXT, MAX_INDEX_LEN, 0);
        return TRUE;
    case WM_COMMAND:
        if (LOWORD(wParam) == IDOK) {
            CardNode *node;
            CardNode *p;
            GetDlgItemText(hDlg, IDD_ADD, buf, sizeof(buf));
            if (buf[0]) {
                node = CreateCardNode(buf, "");
                if (node) {
                    p = node;
                    InsertCardSorted(&p);
                    topcard = node;
                }
                RefreshListBox();
                LoadCardToEdit(topcard);
                EndDialog(hDlg, TRUE);
            }
            return TRUE;
        }
        if (LOWORD(wParam) == IDCANCEL) { EndDialog(hDlg, FALSE); return TRUE; }
        break;
    }
    return FALSE;
}

BOOL CALLBACK IndexDlgProc(HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
    static char buf[MAX_INDEX_LEN+1];
    switch (msg) {
    case WM_INITDIALOG:
        SendDlgItemMessage(hDlg, IDD_INDEX, EM_LIMITTEXT, MAX_INDEX_LEN, 0);
        if (topcard) SetDlgItemText(hDlg, IDD_INDEX, topcard->data.index);
        return TRUE;
    case WM_COMMAND:
        if (LOWORD(wParam) == IDOK) {
            GetDlgItemText(hDlg, IDD_INDEX, buf, sizeof(buf));
            if (buf[0]) {
                if (FindCardByIndex(buf) && lstrcmp(buf, topcard->data.index) != 0) {
                    MessageBox(hDlg, "A card with this index already exists.", "Error", MB_ICONEXCLAMATION);
                    return TRUE;
                }
                SaveCurrentCard();
                {
                    int len = lstrlen(buf);
                    if (len > MAX_INDEX_LEN) len = MAX_INDEX_LEN;
                    memcpy(topcard->data.index, buf, len);
                    topcard->data.index[len] = '\0';
                }
                isModified = TRUE;
                RefreshListBox();
                LoadCardToEdit(topcard);
                EndDialog(hDlg, TRUE);
            }
            return TRUE;
        }
        if (LOWORD(wParam) == IDCANCEL) { EndDialog(hDlg, FALSE); return TRUE; }
        break;
    }
    return FALSE;
}

BOOL CALLBACK AboutDlgProc(HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
    if (msg == WM_COMMAND && (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)) {
        EndDialog(hDlg, TRUE);
        return TRUE;
    }
    return FALSE;
}
