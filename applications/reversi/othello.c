/* othello.c – Windows 3.0, OpenWatcom 1.9, C89 */
#include "othello.h"

/* Глобальные переменные */
static HINSTANCE hInst = NULL;
static HWND      hwndFrame = NULL;
static BOOL      GameOver = FALSE;
static long      lColors;
static HCURSOR   hptrCross, hptrSystem, hptrWait;
static BOARD     Board;
static BOARD     BoardHilf;
static short     sLevel;
static BOOL      fSound = TRUE;
static char      szApp[] = "Othello";
static COLORREF  bgcolor = RGB(0,128,0);
static int       clientHeight = 0;
static BOOL fDisplayText = FALSE;
static char text[129];

static char *pszEvent[] = {
    "Start game", "End game", "New game",
    "Place piece", "Illegal move",
    "Hint", "Pass", "Game won",
    "Game lost", "Draw game"
};

static short sVolumeEve = 100;

/* ================================================================
 * Обработка фатальной ошибки
 * ================================================================ */
void Error(BOOL fMessage)
{
    if(fMessage)
#ifdef GERMAN
        MessageBox(HWND_DESKTOP, "Nicht behebbarer Anwendungsfehler.", szApp, MB_OK | MB_ICONHAND);
#else
        MessageBox(HWND_DESKTOP, "Unrecoverable application error.", szApp, MB_OK | MB_ICONHAND);
#endif
    else
        MessageBeep(0);

    if (hwndFrame != NULL)
        DestroyWindow(hwndFrame);
    PostQuitMessage(RETURN_ERROR);
}

/* ================================================================
 * WinMain – точка входа
 * ================================================================ */
int PASCAL WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
    MSG msg;
    WNDCLASS wc;
    RECT rect;
    int cxScreen, cyScreen;
    HACCEL hAccel;

    hInst = hInstance;

    hptrSystem = LoadCursor(NULL, IDC_ARROW);
    hptrWait   = LoadCursor(NULL, IDC_WAIT);
    hptrCross  = LoadCursor(NULL, IDC_CROSS);
    if (!hptrCross) hptrCross = hptrSystem;

    {
        HDC hdc = GetDC(NULL);
        lColors = GetDeviceCaps(hdc, NUMCOLORS);
        ReleaseDC(NULL, hdc);
    }

    wc.style         = CS_HREDRAW | CS_VREDRAW;
    wc.lpfnWndProc   = WndProc;
    wc.cbClsExtra    = 0;
    wc.cbWndExtra    = 0;
    wc.hInstance     = hInstance;
    wc.hIcon         = LoadIcon(hInstance, MAKEINTRESOURCE(WD_MAIN));
    wc.hCursor       = hptrSystem;
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW+1);
    wc.lpszMenuName  = MAKEINTRESOURCE(WD_MAIN);
    wc.lpszClassName = szApp;

    if (!RegisterClass(&wc))
        return RETURN_ERROR;

    cxScreen = GetSystemMetrics(SM_CXSCREEN);
    cyScreen = GetSystemMetrics(SM_CYSCREEN);
    rect.left = (cxScreen - 400) / 2;
    rect.top  = (cyScreen - 350) / 2;
    rect.right  = rect.left + 400;
    rect.bottom = rect.top  + 350;
    AdjustWindowRect(&rect, WS_OVERLAPPEDWINDOW, TRUE);

    hwndFrame = CreateWindow(szApp, szApp, WS_OVERLAPPEDWINDOW,
                             rect.left, rect.top,
                             rect.right - rect.left, rect.bottom - rect.top,
                             NULL, NULL, hInstance, NULL);
    if (!hwndFrame)
        return RETURN_ERROR;

    /* Загружаем акселераторы */
    hAccel = LoadAccelerators(hInstance, MAKEINTRESOURCE(WD_MAIN));

    ShowWindow(hwndFrame, nCmdShow);
    UpdateWindow(hwndFrame);

    while (GetMessage(&msg, NULL, 0, 0)) {
        if (!TranslateAccelerator(hwndFrame, hAccel, &msg)) {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }

    return msg.wParam;
}
/* ================================================================
 * WndProc – главная оконная процедура
 * ================================================================ */
LRESULT WINAPI WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    POINT point;
    short sX, sY, sPlayer, sComputer;
//    static char text[129];
    static BOOL fPtrInstalled;
    static short cxBlock, cyBlock;
    static BOOL fPlayerStarts;
//    static BOOL fDisplayText;
    static BOOL fFirst;
    RECT rect;
    HDC hdc;
    PAINTSTRUCT ps;

    switch(msg)
    {
    case WM_CREATE: {
        char szBuf[16];
        fFirst = TRUE;
        fDisplayText = TRUE;
        if (lColors == 2)
#ifdef GERMAN
            strcpy(text, "Sie spielen mit den schwarzen Steinen.");
        else
            strcpy(text, "Sie spielen mit den blauen Steinen.");
#else
            strcpy(text, "You place the black markers.");
        else
            strcpy(text, "You place the blue markers.");
#endif

        GameOver = FALSE;
        for(sX = 0; sX < 8; sX++)
            for(sY = 0; sY < 8; sY++)
                Board.sField[sX][sY] = EMPTY;
        Board.sField[3][3] = PLAYER;
        Board.sField[4][4] = PLAYER;
        Board.sField[3][4] = COMPUTER;
        Board.sField[4][3] = COMPUTER;

        GetProfileString(szApp, "BGColor", "0x00800000", szBuf, sizeof(szBuf));
        sscanf(szBuf, "%lx", &bgcolor);
        sVolumeEve = GetProfileInt(szApp, "Volume", 100);
        fPlayerStarts = GetProfileInt(szApp, "PlayerStarts", TRUE);
        fSound = GetProfileInt(szApp, "Sound", TRUE);
        sLevel = abs(GetProfileInt(szApp, "Level", 0)) % 3;

        fPtrInstalled = GetSystemMetrics(SM_MOUSEPRESENT);
        PostMessage(hwnd, WM_COMMAND, MI_NEW, 0);
        if(fSound) {
            PSOUNDEVENT pSE = malloc(sizeof(SOUNDEVENT));
            if(pSE) {
                pSE->sEvent = 0;
                vdGetWavFile(0, pSE->chWavFile);
                vdPlayWavFileAsyncEve(pSE);
            }
        }
        return 0;
    }

    case WM_SIZE:
        cxBlock = LOWORD(lParam) / (DIVISIONS + 2);
        cyBlock = HIWORD(lParam) / (DIVISIONS + 2);
    {
        RECT rc;
        GetClientRect(hwnd, &rc);
        clientHeight = rc.bottom - rc.top;
    }
        break;

    case WM_MOUSEMOVE:
    if (GameOver) break;
    if (cxBlock > 0 && cyBlock > 0) {
        /* Переводим Windows-координаты в индексы OS/2 */
        sX = LOWORD(lParam) / cxBlock;
        sY = (clientHeight - HIWORD(lParam)) / cyBlock;
        SetPointer(sX, sY);
    }
    return 0;

    case WM_CHAR:
        if (GameOver) return 0;
        if (cxBlock <= 0 || cyBlock <= 0) return 0;
        {
            POINT pt;
            GetCursorPos(&pt);
            ScreenToClient(hwnd, &pt);
            sX = pt.x / cxBlock;
	    sY = (clientHeight - pt.y) / cyBlock;
            sX = max(0, min(DIVISIONS + 2, sX));
            sY = max(0, min(DIVISIONS + 2, sY));
        }
        if (wParam == VK_SPACE || wParam == VK_RETURN) {
            SendMessage(hwnd, WM_USER, 0, MAKELPARAM(sX, sY));
            return 0;
        }
        if (wParam == VK_UP) sY++;
        else if (wParam == VK_DOWN) sY--;
        else if (wParam == VK_LEFT) sX--;
        else if (wParam == VK_RIGHT) sX++;
        else if (wParam == VK_HOME) { sX = 1; sY = 8; }
        else if (wParam == VK_END)  { sX = 8; sY = 1; }
        else return DefWindowProc(hwnd, msg, wParam, lParam);

        if (sX == 0) sX = DIVISIONS;
        else if (sX == DIVISIONS + 1) sX = DIVISIONS;
        if (sY == 0) sY = DIVISIONS;
        else if (sY == DIVISIONS + 1) sY = DIVISIONS;

        point.x = sX * cxBlock + cxBlock / 2;
        point.y = sY * cyBlock + cyBlock / 2;
        ClientToScreen(hwnd, &point);
        SetCursorPos(point.x, point.y);
        return 0;

    case WM_SETFOCUS:
        if (!fPtrInstalled)
            ShowCursor((BOOL)wParam);
        break;

    case WM_LBUTTONUP:
    if (GameOver) break;
    if (cxBlock > 0 && cyBlock > 0) {
        sX = LOWORD(lParam) / cxBlock;
        sY = (clientHeight - HIWORD(lParam)) / cyBlock;
        SendMessage(hwnd, WM_USER, 0, MAKELPARAM(sX, sY));
    }
    break;

case WM_USER: {
    int left, top, right, bottom;
    if (GameOver) break;
    sX = LOWORD(lParam);
    sY = HIWORD(lParam);

    /* Очистка сообщения "I must pass", если оно было */
    if (fDisplayText) {
        fDisplayText = FALSE;
        left   = 0;
        top    = clientHeight - ((DIVISIONS + 3) * cyBlock);
        right  = (DIVISIONS + 3) * cxBlock;
        bottom = clientHeight - ((DIVISIONS + 1) * cyBlock);
        SetRect(&rect, left, top, right, bottom);
        InvalidateRect(hwnd, &rect, FALSE);
    }

    if (sX == 0 || sX == 9 || sY == 0 || sY == 9) {
        if(fSound) {
            PSOUNDEVENT pSE = malloc(sizeof(SOUNDEVENT));
            if(pSE) { pSE->sEvent = 4; vdGetWavFile(4, pSE->chWavFile); vdPlayWavFileAsyncEve(pSE); }
        }
        break;
    }

    sX--; sY--;
    if(!fIsMovePossible(Board, sX, sY, PLAYER)) {
        if(fSound) {
            PSOUNDEVENT pSE = malloc(sizeof(SOUNDEVENT));
            if(pSE) { pSE->sEvent = 4; vdGetWavFile(4, pSE->chWavFile); vdPlayWavFileAsyncEve(pSE); }
        }
        break;
    }

    if(fSound) {
        PSOUNDEVENT pSE = malloc(sizeof(SOUNDEVENT));
        if(pSE) { pSE->sEvent = 3; vdGetWavFile(3, pSE->chWavFile); vdPlayWavFileAsyncEve(pSE); }
    }
    MakeMove(&Board, sX, sY, PLAYER, TRUE, TRUE, cxBlock, cyBlock, hwnd);

    /* Проверка конца игры после хода игрока */
    if (fGameOver(Board)) {
        GameOver = TRUE;
        if (cxBlock > 0 && cyBlock > 0) {
            GetCursorPos(&point);
            ScreenToClient(hwnd, &point);
            point.x /= cxBlock;
            point.y /= cyBlock;
            SetPointer(point.x, point.y);
        }
        Result(Board, &sComputer, &sPlayer);
        if(sComputer == sPlayer) {
            if(fSound) {
                PSOUNDEVENT pSE = malloc(sizeof(SOUNDEVENT));
                if(pSE) { pSE->sEvent = 9; vdGetWavFile(9, pSE->chWavFile); vdPlayWavFileAsyncEve(pSE); }
            }
#ifdef GERMAN
            strcpy(text, "Das Spiel ist unentschieden.");
#else
            strcpy(text, "The game is a draw.");
#endif
        } else if(sComputer > sPlayer) {
            if(fSound) {
                PSOUNDEVENT pSE = malloc(sizeof(SOUNDEVENT));
                if(pSE) { pSE->sEvent = 8; vdGetWavFile(8, pSE->chWavFile); vdPlayWavFileAsyncEve(pSE); }
            }
#ifdef GERMAN
            sprintf(text, "Sie haben um %d Punkte verloren.", sComputer - sPlayer);
#else
            sprintf(text, "You lost by %d.", sComputer - sPlayer);
#endif
        } else {
            if(fSound) {
                PSOUNDEVENT pSE = malloc(sizeof(SOUNDEVENT));
                if(pSE) { pSE->sEvent = 7; vdGetWavFile(7, pSE->chWavFile); vdPlayWavFileAsyncEve(pSE); }
            }
#ifdef GERMAN
            sprintf(text, "Sie haben um %d Punkte gewonnen.", sPlayer - sComputer);
#else
            sprintf(text, "You won by %d.", sPlayer - sComputer);
#endif
        }
        left   = 0;
        top    = clientHeight - ((DIVISIONS + 3) * cyBlock);
        right  = (DIVISIONS + 3) * cxBlock;
        bottom = clientHeight - ((DIVISIONS + 1) * cyBlock);
        SetRect(&rect, left, top, right, bottom);
        InvalidateRect(hwnd, &rect, FALSE);
        UpdateWindow(hwnd);
        break;
    }

    /* Компьютер пасует – игра продолжается */
    if (fMustPass(Board, COMPUTER)) {
        if(fSound) {
            PSOUNDEVENT pSE = malloc(sizeof(SOUNDEVENT));
            if(pSE) { pSE->sEvent = 6; vdGetWavFile(6, pSE->chWavFile); vdPlayWavFileAsyncEve(pSE); }
        }
        fDisplayText = TRUE;
        strcpy(text, "I must pass.");
        left   = 0;
        top    = clientHeight - ((DIVISIONS + 3) * cyBlock);
        right  = (DIVISIONS + 3) * cxBlock;
        bottom = clientHeight - ((DIVISIONS + 1) * cyBlock);
        SetRect(&rect, left, top, right, bottom);
        InvalidateRect(hwnd, &rect, FALSE);
        if (cxBlock > 0 && cyBlock > 0) {
            GetCursorPos(&point);
            ScreenToClient(hwnd, &point);
            point.x /= cxBlock;
            point.y /= cyBlock;
            SetPointer(point.x, point.y);
        }
        /* Игра не заканчивается, ждём следующего хода игрока */
    } else {
        SetCursor(hptrWait);
        DoComputerMove(hwnd, FALSE);
        SetCursor(hptrSystem);
    }
    break;
}
    case WM_COMMAND:
        switch (LOWORD(wParam)) {
        case MI_COLOR0:  bgcolor = RGB(0,0,0);       InvalidateRect(hwnd, NULL, FALSE); break;
        case MI_COLOR1:  bgcolor = RGB(0,0,255);     InvalidateRect(hwnd, NULL, FALSE); break;
        case MI_COLOR2:  bgcolor = RGB(255,0,0);     InvalidateRect(hwnd, NULL, FALSE); break;
        case MI_COLOR3:  bgcolor = RGB(255,0,255);   InvalidateRect(hwnd, NULL, FALSE); break;
        case MI_COLOR4:  bgcolor = RGB(0,255,0);     InvalidateRect(hwnd, NULL, FALSE); break;
        case MI_COLOR5:  bgcolor = RGB(0,255,255);   InvalidateRect(hwnd, NULL, FALSE); break;
        case MI_COLOR6:  bgcolor = RGB(255,255,0);   InvalidateRect(hwnd, NULL, FALSE); break;
        case MI_COLOR7:  bgcolor = RGB(255,255,255); InvalidateRect(hwnd, NULL, FALSE); break;
        case MI_COLOR8:  bgcolor = RGB(128,128,128); InvalidateRect(hwnd, NULL, FALSE); break;
        case MI_COLOR9:  bgcolor = RGB(0,0,128);     InvalidateRect(hwnd, NULL, FALSE); break;
        case MI_COLOR10: bgcolor = RGB(128,0,0);     InvalidateRect(hwnd, NULL, FALSE); break;
        case MI_COLOR11: bgcolor = RGB(128,0,128);   InvalidateRect(hwnd, NULL, FALSE); break;
        case MI_COLOR12: bgcolor = RGB(0,128,0);     InvalidateRect(hwnd, NULL, FALSE); break;
        case MI_COLOR13: bgcolor = RGB(0,128,128);   InvalidateRect(hwnd, NULL, FALSE); break;
        case MI_COLOR14: bgcolor = RGB(128,128,0);   InvalidateRect(hwnd, NULL, FALSE); break;
        case MI_COLOR15: bgcolor = RGB(192,192,192); InvalidateRect(hwnd, NULL, FALSE); break;

        case MI_BEGINNER: sLevel = 0; break;
        case MI_ADVANCED: sLevel = 1; break;
        case MI_MASTER:   sLevel = 2; break;
        case MI_PLAYER:   fPlayerStarts = TRUE;  SendMessage(hwnd, WM_COMMAND, MI_NEW, 0); break;
        case MI_COMPUTER: fPlayerStarts = FALSE; SendMessage(hwnd, WM_COMMAND, MI_NEW, 0); break;

        case MI_SOUND:
	{
		FARPROC lpDlg = MakeProcInstance((FARPROC)dpEve, hInst);
	        DialogBox(hInst, MAKEINTRESOURCE(DB_EVE), hwnd, (DLGPROC)lpDlg);
		FreeProcInstance(lpDlg);
	}
            break;

        case MI_HINT:
            if (GameOver) break;
            if(fSound) {
                PSOUNDEVENT pSE = malloc(sizeof(SOUNDEVENT));
                if(pSE) { pSE->sEvent = 5; vdGetWavFile(5, pSE->chWavFile); vdPlayWavFileAsyncEve(pSE); }
            }
            for(sX = 0; sX < 8; sX++)
                for(sY = 0; sY < 8; sY++) {
                    if(Board.sField[sX][sY] == COMPUTER)
                        BoardHilf.sField[sX][sY] = PLAYER;
                    else if(Board.sField[sX][sY] == PLAYER)
                        BoardHilf.sField[sX][sY] = COMPUTER;
                    else
                        BoardHilf.sField[sX][sY] = EMPTY;
                }
            SetCursor(hptrWait);
            DoComputerMove(hwnd, TRUE);
            SetCursor(hptrSystem);
            break;

        case MI_NEW:
            if(!fFirst && fSound) {
                PSOUNDEVENT pSE = malloc(sizeof(SOUNDEVENT));
                if(pSE) { pSE->sEvent = 2; vdGetWavFile(2, pSE->chWavFile); vdPlayWavFileAsyncEve(pSE); }
            }
            fFirst = FALSE;
            GameOver = FALSE;
            for(sX = 0; sX < 8; sX++)
                for(sY = 0; sY < 8; sY++)
                    Board.sField[sX][sY] = EMPTY;
            Board.sField[3][3] = PLAYER;
            Board.sField[4][4] = PLAYER;
            Board.sField[3][4] = COMPUTER;
            Board.sField[4][3] = COMPUTER;
            InvalidateRect(hwnd, NULL, FALSE);
            if (cxBlock > 0 && cyBlock > 0) {
                GetCursorPos(&point);
                ScreenToClient(hwnd, &point);
                point.x /= cxBlock;
                point.y /= cyBlock;
                SetPointer(point.x, point.y);
            }
            if(!fPlayerStarts) {
                SetCursor(hptrWait);
                DoComputerMove(hwnd, FALSE);
                SetCursor(hptrSystem);
            }
            break;

        case MI_HELPPRODUCTINFO:
		{
		FARPROC lpDlg = MakeProcInstance((FARPROC)dpProdInfo, hInst);
	        DialogBox(hInst, MAKEINTRESOURCE(DB_PRODINFO), hwnd, (DLGPROC)lpDlg);
		FreeProcInstance(lpDlg);
		}
            break;

        default:
            return DefWindowProc(hwnd, msg, wParam, lParam);
        }
        break;

    case WM_INITMENU: {
        HMENU hMenu = (HMENU)wParam;
        EnableMenuItem(hMenu, MI_NEW, MF_ENABLED);
        EnableMenuItem(hMenu, MI_HINT, GameOver ? MF_GRAYED : MF_ENABLED);
        CheckMenuItem(hMenu, MI_BEGINNER, (sLevel==0) ? MF_CHECKED : MF_UNCHECKED);
        CheckMenuItem(hMenu, MI_ADVANCED, (sLevel==1) ? MF_CHECKED : MF_UNCHECKED);
        CheckMenuItem(hMenu, MI_MASTER,   (sLevel==2) ? MF_CHECKED : MF_UNCHECKED);
        EnableMenuItem(hMenu, MI_PLAYER,   (!fPlayerStarts) ? MF_ENABLED : MF_GRAYED);
        EnableMenuItem(hMenu, MI_COMPUTER, fPlayerStarts  ? MF_ENABLED : MF_GRAYED);
        return 0;
    }

    case WM_ERASEBKGND:
        return 1;

    case WM_DESTROY: {
        char buf[32];
        sprintf(buf, "0x%06lx", bgcolor);
        WriteProfileString(szApp, "BGColor", buf);
        sprintf(buf, "%d", sVolumeEve);
        WriteProfileString(szApp, "Volume", buf);
        WriteProfileString(szApp, "PlayerStarts", fPlayerStarts ? "1" : "0");
        WriteProfileString(szApp, "Sound", fSound ? "1" : "0");
        sprintf(buf, "%d", sLevel);
        WriteProfileString(szApp, "Level", buf);
        DestroyCursor(hptrCross);
        PostQuitMessage(0);
        break;
    }

case WM_PAINT: {
    RECT rcClient, src, dest, rText, chip;
    int sOffset;
    HBRUSH hBrushBg, hBrDark, hBrBlack, hOldBr;
    HPEN hPen, hOldPen;
    int left, right, top, bottom;
    int w, h;
    COLORREF clr;
    POINT pt[4];
    int sX, sY;

    GetClientRect(hwnd, &rcClient);
    clientHeight = rcClient.bottom - rcClient.top;

    hdc = BeginPaint(hwnd, &ps);

    /* ===== Фон (четыре полосы, координаты строго из OS/2) ===== */
    hBrushBg = CreateSolidBrush(bgcolor);

    left   = 0;
    top    = clientHeight - ((DIVISIONS + 3) * cyBlock);
    right  = cxBlock;
    bottom = clientHeight;
    SetRect(&src, left, top, right, bottom);
    if (IntersectRect(&dest, &src, &ps.rcPaint))
        FillRect(hdc, &src, hBrushBg);

    left   = cxBlock;
    top    = clientHeight - ((DIVISIONS + 3) * cyBlock);
    right  = (DIVISIONS + 3) * cxBlock;
    bottom = clientHeight - ((DIVISIONS + 1) * cyBlock + 1);
    SetRect(&src, left, top, right, bottom);
    if (IntersectRect(&dest, &src, &ps.rcPaint))
        FillRect(hdc, &src, hBrushBg);

    left   = (DIVISIONS + 1) * cxBlock + 1;
    top    = clientHeight - ((DIVISIONS + 2) * cyBlock);
    right  = (DIVISIONS + 3) * cxBlock;
    bottom = clientHeight;
    SetRect(&src, left, top, right, bottom);
    if (IntersectRect(&dest, &src, &ps.rcPaint))
        FillRect(hdc, &src, hBrushBg);

    left   = cxBlock;
    top    = clientHeight - (cyBlock + 1);
    right  = (DIVISIONS + 1) * cxBlock + 1;
    bottom = clientHeight;
    SetRect(&src, left, top, right, bottom);
    if (IntersectRect(&dest, &src, &ps.rcPaint))
        FillRect(hdc, &src, hBrushBg);

    DeleteObject(hBrushBg);

    /* ===== Сетка (OS/2 -> Windows Y) ===== */
    hPen = CreatePen(PS_SOLID, 1, RGB(192, 192, 192));
    hOldPen = SelectObject(hdc, hPen);
    for (sX = 1; sX <= DIVISIONS; sX++) {
        for (sY = 1; sY <= DIVISIONS; sY++) {
            left   = sX * cxBlock;
            top    = clientHeight - ((sY + 1) * cyBlock + 1);
            right  = (sX + 1) * cxBlock + 1;
            bottom = clientHeight - (sY * cyBlock);
            SetRect(&src, left, top, right, bottom);
            if (IntersectRect(&dest, &src, &ps.rcPaint))
                Rectangle(hdc, src.left, src.top, src.right, src.bottom);
        }
    }
    SelectObject(hdc, hOldPen);
    DeleteObject(hPen);

    /* ===== Рамки (тёмно-серые полигоны, исправлен clipping rect) ===== */
    hBrDark = CreateSolidBrush(RGB(128, 128, 128));

    /* Верхняя рамка (на самом деле нижняя в Windows, но соответствует оригиналу) */
    pt[0].x = cxBlock;
    pt[0].y = clientHeight - cyBlock;                         /* OS/2 y = cyBlock        -> низ */
    pt[1].x = cxBlock + cxBlock / 4;
    pt[1].y = clientHeight - (cyBlock - cyBlock / 4);        /* OS/2 y = cyBlock-cyBlock/4 -> чуть выше */
    pt[2].x = (DIVISIONS + 1) * cxBlock + cxBlock / 4;
    pt[2].y = pt[1].y;
    pt[3].x = (DIVISIONS + 1) * cxBlock;
    pt[3].y = pt[0].y;
    /* Правильный прямоугольник: top = pt[3].y (меньший Y), bottom = pt[1].y (больший) */
    SetRect(&src, pt[1].x, pt[3].y, pt[2].x, pt[1].y);
    if (IntersectRect(&dest, &src, &ps.rcPaint)) {
        hOldBr = SelectObject(hdc, hBrDark);
        Polygon(hdc, pt, 4);
        SelectObject(hdc, hOldBr);
    }

    /* Правая рамка */
    pt[0].x = (DIVISIONS + 1) * cxBlock;
    pt[0].y = clientHeight - (DIVISIONS + 1) * cyBlock;                  /* верх */
    pt[1].x = pt[0].x + cxBlock / 4;
    pt[1].y = clientHeight - ((DIVISIONS + 1) * cyBlock - cyBlock / 4); /* чуть ниже */
    pt[2].x = pt[1].x;
    pt[2].y = clientHeight - (cyBlock - cyBlock / 4);                   /* низ */
    pt[3].x = pt[0].x;
    pt[3].y = clientHeight - cyBlock;                                    /* низ */
    /* Правильный прямоугольник: top = pt[0].y (меньший), bottom = pt[2].y (больший) */
    SetRect(&src, pt[3].x, pt[0].y, pt[1].x, pt[2].y);
    if (IntersectRect(&dest, &src, &ps.rcPaint)) {
        hOldBr = SelectObject(hdc, hBrDark);
        Polygon(hdc, pt, 4);
        SelectObject(hdc, hOldBr);
    }
    DeleteObject(hBrDark);

    /* ===== Тени (чёрные) ===== */
    hBrBlack = CreateSolidBrush(RGB(0, 0, 0));
    left   = cxBlock + cxBlock / 2;
    top    = clientHeight - (3 * cyBlock / 4 + 1);
    right  = (DIVISIONS + 1) * cxBlock + cxBlock / 2 + 1;
    bottom = clientHeight - (cyBlock / 2);
    SetRect(&src, left, top, right, bottom);
    if (IntersectRect(&dest, &src, &ps.rcPaint))
        FillRect(hdc, &src, hBrBlack);

    left   = (DIVISIONS + 1) * cxBlock + cxBlock / 4;
    top    = clientHeight - ((DIVISIONS + 1) * cyBlock - cyBlock / 2 + 1);
    right  = (DIVISIONS + 1) * cxBlock + cxBlock / 2 + 1;
    bottom = clientHeight - (cyBlock / 2);
    SetRect(&src, left, top, right, bottom);
    if (IntersectRect(&dest, &src, &ps.rcPaint))
        FillRect(hdc, &src, hBrBlack);
    DeleteObject(hBrBlack);

    /* ===== Фишки ===== */
    sOffset = (cxBlock < cyBlock ? cxBlock : cyBlock) / 10;
    for (sX = 1; sX <= DIVISIONS; sX++) {
        for (sY = 1; sY <= DIVISIONS; sY++) {
            if (Board.sField[sX - 1][sY - 1] != EMPTY) {
                left   = sX * cxBlock;
                top    = clientHeight - ((sY + 1) * cyBlock);
                right  = (sX + 1) * cxBlock;
                bottom = clientHeight - (sY * cyBlock);
                SetRect(&src, left, top, right, bottom);
                if (IntersectRect(&dest, &src, &ps.rcPaint)) {
                    if (Board.sField[sX - 1][sY - 1] == COMPUTER)
                        clr = (lColors == 2) ? RGB(255, 255, 255) : RGB(255, 0, 0);
                    else
                        clr = (lColors == 2) ? RGB(0, 0, 0) : RGB(0, 0, 255);

                    chip = src;
                    w = chip.right - chip.left;
                    h = chip.bottom - chip.top;
                    if (w < h) {
                        chip.top += (h - w) / 2;
                        chip.bottom -= (h - w) / 2;
                    } else if (h < w) {
                        chip.left += (w - h) / 2;
                        chip.right -= (w - h) / 2;
                    }
                    chip.left   += sOffset;
                    chip.top    += sOffset;
                    chip.right  -= sOffset;
                    chip.bottom -= sOffset;

                    hOldBr = SelectObject(hdc, CreateSolidBrush(clr));
                    Ellipse(hdc, chip.left, chip.top, chip.right, chip.bottom);
                    DeleteObject(SelectObject(hdc, hOldBr));
                }
            }
        }
    }

    /* ===== Текстовая строка ===== */
    if (GameOver || fDisplayText) {
        left   = 0;
        top    = clientHeight - ((DIVISIONS + 3) * cyBlock);
        right  = (DIVISIONS + 3) * cxBlock;
        bottom = clientHeight - ((DIVISIONS + 1) * cyBlock);
        SetRect(&rText, left, top, right, bottom);
        SetBkMode(hdc, TRANSPARENT);
        SetTextColor(hdc, (bgcolor != RGB(255, 255, 255)) ? RGB(255, 255, 255) : RGB(0, 0, 0));
        DrawText(hdc, text, -1, &rText, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
    }

    EndPaint(hwnd, &ps);
    break;
}

    case WM_CLOSE:
        DestroyWindow(hwnd);
        break;

    default:
        return DefWindowProc(hwnd, msg, wParam, lParam);
    }
    return 0;
}

/* ================================================================
 * Вспомогательные функции
 * ================================================================ */
void SetPointer(short sX, short sY)
{
    if (GameOver || sX == 0 || sY == 0 || sX == 9 || sY == 9)
        SetCursor(hptrSystem);
    else if(fIsMovePossible(Board, sX-1, sY-1, PLAYER))
        SetCursor(hptrCross);
    else
        SetCursor(hptrSystem);
}

void DisplayMessage(const char *pchStr)
{
    MessageBox(HWND_DESKTOP, pchStr, szApp, MB_OK | MB_ICONHAND);
}

void Result(BOARD Board, short *psComputer, short *psPlayer)
{
    short sX, sY, sumPlayer = 0, sumComputer = 0;
    for(sX = 0; sX < 8; sX++)
        for(sY = 0; sY < 8; sY++) {
            if(Board.sField[sX][sY] == PLAYER) sumPlayer++;
            else if(Board.sField[sX][sY] == COMPUTER) sumComputer++;
        }
    *psComputer = sumComputer;
    *psPlayer = sumPlayer;
}

void DoComputerMove(HWND hwnd, BOOL fHint)
{
    short sX, sY;
    BOOL fValid;
    RECT rc;
    short cX, cY;
    POINT pt;
    short sComputer, sPlayer;
    int localHeight;
    int left, top, right, bottom;

    if (fHint)
        sOthello(BoardHilf, 0, 0, 0, sLevel+2, COMPUTER, &sX, &sY, &fValid, 0);
    else
        sOthello(Board, 0, 0, 0, sLevel+2, COMPUTER, &sX, &sY, &fValid, 0);

    if (fHint && fValid) {
        /* Подсказка (уже исправлена) */
        int top_y, bottom_y;
        GetClientRect(hwnd, &rc);
        cX = (short)((rc.right - rc.left) / (DIVISIONS + 2));
        cY = (short)((rc.bottom - rc.top) / (DIVISIONS + 2));
        left   = (sX + 1) * cX;
        top_y  = clientHeight - ((sY + 2) * cY);
        bottom_y = clientHeight - ((sY + 1) * cY);
        pt.x = left + cX / 2;
        pt.y = (top_y + bottom_y) / 2;
        ClientToScreen(hwnd, &pt);
        SetCursorPos(pt.x, pt.y);
        return;
    }

    if (!fHint && fValid) {
        if(fSound) {
            PSOUNDEVENT pSE = malloc(sizeof(SOUNDEVENT));
            if(pSE) { pSE->sEvent = 3; vdGetWavFile(3, pSE->chWavFile); vdPlayWavFileAsyncEve(pSE); }
        }
        GetClientRect(hwnd, &rc);
        localHeight = rc.bottom - rc.top;
        cX = (short)((rc.right - rc.left) / (DIVISIONS + 2));
        cY = (short)((rc.bottom - rc.top) / (DIVISIONS + 2));
        MakeMove(&Board, sX, sY, COMPUTER, TRUE, TRUE, cX, cY, hwnd);

        /* Проверка конца игры */
        if (fGameOver(Board)) {
            GameOver = TRUE;
            Result(Board, &sComputer, &sPlayer);
            if(sComputer == sPlayer) {
                strcpy(text, "The game is a draw.");
            } else if(sComputer > sPlayer) {
                sprintf(text, "You lost by %d.", sComputer - sPlayer);
            } else {
                sprintf(text, "You won by %d.", sPlayer - sComputer);
            }
            /* Инвалидация текстовой области */
            left   = 0;
            top    = localHeight - ((DIVISIONS + 3) * cY);
            right  = (DIVISIONS + 3) * cX;
            bottom = localHeight - ((DIVISIONS + 1) * cY);
            SetRect(&rc, left, top, right, bottom);
            InvalidateRect(hwnd, &rc, FALSE);
            UpdateWindow(hwnd);
            return;
        }

        /* Если игрок должен пасовать – ход компьютера повторяется */
        if (fMustPass(Board, PLAYER)) {
            fDisplayText = TRUE;
            strcpy(text, "You must pass.");
            left   = 0;
            top    = localHeight - ((DIVISIONS + 3) * cY);
            right  = (DIVISIONS + 3) * cX;
            bottom = localHeight - ((DIVISIONS + 1) * cY);
            SetRect(&rc, left, top, right, bottom);
            InvalidateRect(hwnd, &rc, FALSE);
            UpdateWindow(hwnd);

            /* Пауза и обработка сообщений */
            {
                DWORD start = GetTickCount();
                while (GetTickCount() - start < 500) {
                    MSG msg;
                    if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
                        TranslateMessage(&msg);
                        DispatchMessage(&msg);
                    }
                }
            }

            /* Рекурсивный ход компьютера */
            SetCursor(hptrWait);
            DoComputerMove(hwnd, FALSE);
            SetCursor(hptrSystem);
            return;
        }
    }
    InvalidateRect(hwnd, NULL, FALSE);
}

void vdGetWavFile(unsigned short usEvent, char *pszWavFile)
{
    pszWavFile[0] = '\0';
}

void vdPlayWavFileAsyncEve(PSOUNDEVENT pSE)
{
    if (!fSound || !pSE) {
        free(pSE);
        return;
    }
    MessageBeep(0);
    free(pSE);
}

/* ================================================================
 * Диалоговые процедуры
 * ================================================================ */
BOOL WINAPI dpProdInfo(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    switch(msg) {
    case WM_INITDIALOG: return TRUE;
    case WM_COMMAND: EndDialog(hwnd, TRUE); return TRUE;
    }
    return FALSE;
}

BOOL WINAPI dpEve(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    switch(msg) {
    case WM_INITDIALOG:
        CheckDlgButton(hwnd, CB_EVESOUNDS, fSound ? BST_CHECKED : BST_UNCHECKED);
        return TRUE;
    case WM_COMMAND:
        if (LOWORD(wParam) == DID_OK || LOWORD(wParam) == IDCANCEL) {
            fSound = IsDlgButtonChecked(hwnd, CB_EVESOUNDS);
            EndDialog(hwnd, LOWORD(wParam) == DID_OK);
            return TRUE;
        }
        break;
    }
    return FALSE;
}

/* ================================================================
 * ИГРОВАЯ ЛОГИКА (полная, без сокращений)
 * ================================================================ */

static short asFlipped[8][8] = {
    {0, 255, 0, 5, 5, 0, 255, 0},
    {255, 0, -10, 2, 2, -10, 0, 255},
    {0, -10, -5, 2, 2, -5, -10, 0},
    {5, 2, 2, 20, 20, 2, 2, 5},
    {5, 2, 2, 20, 20, 2, 2, 5},
    {0, -10, -5, 2, 2, -5, -10, 0},
    {255, 0, -10, 2, 2, -10, 0, 255},
    {0, 255, 0, 5, 5, 0, 255, 0}
};

short sSquare(BOARD Board, short sOwn, short sOpp, short sX, short sY)
{
    if (sX == 0 && sY == 7) {
        if (Board.sField[1][7] == EMPTY && Board.sField[2][7] == sOwn && Board.sField[2][6] == sOwn) {
            if (Board.sField[6][7] == sOwn) return 100; else return 150;
        }
        if (Board.sField[0][6] == EMPTY && Board.sField[0][5] == sOwn && Board.sField[1][5] == sOwn) {
            if (Board.sField[0][1] == sOwn) return 100; else return 150;
        }
        if (Board.sField[1][7] != EMPTY || Board.sField[0][6] != EMPTY) return 400;
        return 255;
    }
    if (sX == 0 && sY == 0) {
        if (Board.sField[1][0] == EMPTY && Board.sField[2][0] == sOwn && Board.sField[2][1] == sOwn) {
            if (Board.sField[6][0] == sOwn) return 100; else return 150;
        }
        if (Board.sField[0][1] == EMPTY && Board.sField[0][2] == sOwn && Board.sField[1][2] == sOwn) {
            if (Board.sField[0][6] == sOwn) return 100; else return 150;
        }
        if (Board.sField[1][0] != EMPTY || Board.sField[0][1] != EMPTY) return 400;
        return 255;
    }
    if (sX == 7 && sY == 0) {
        if (Board.sField[6][0] == EMPTY && Board.sField[5][0] == sOwn && Board.sField[5][1] == sOwn) {
            if (Board.sField[1][0] == sOwn) return 100; else return 150;
        }
        if (Board.sField[7][1] == EMPTY && Board.sField[7][2] == sOwn && Board.sField[6][2] == sOwn) {
            if (Board.sField[7][6] == sOwn) return 100; else return 150;
        }
        if (Board.sField[6][0] != EMPTY || Board.sField[7][1] != EMPTY) return 400;
        return 255;
    }
    if (sX == 7 && sY == 7) {
        if (Board.sField[6][7] == EMPTY && Board.sField[5][7] == sOwn && Board.sField[5][6] == sOwn) {
            if (Board.sField[1][7] == sOwn) return 100; else return 150;
        }
        if (Board.sField[7][6] == EMPTY && Board.sField[7][5] == sOwn && Board.sField[6][5] == sOwn) {
            if (Board.sField[7][1] == sOwn) return 100; else return 150;
        }
        if (Board.sField[6][7] != EMPTY || Board.sField[7][6] != EMPTY) return 400;
        return 255;
    }

    /* C squares */
    if (sX == 1 && sY == 7) {
        if (Board.sField[0][7] == sOwn) return 175;
        if (Board.sField[0][7] == sOpp) return 2;
        if (Board.sField[2][7] == EMPTY) return -150;
        if (Board.sField[2][7] == sOpp) return -100;
        if (Board.sField[3][7] == EMPTY) return -150;
        if (Board.sField[3][7] == sOpp) return -100;
        if (Board.sField[4][7] == EMPTY) {
            if (Board.sField[3][6] == sOpp) return 10;
            else return -150;
        }
        if (Board.sField[4][7] == sOpp) return -100;
        if (Board.sField[5][7] == EMPTY) return 10;
        if (Board.sField[5][7] == sOpp) return -100;
        if (Board.sField[6][7] == EMPTY) {
            if (Board.sField[5][6] == sOwn) return -225;
            else return 10;
        }
        if (Board.sField[6][7] == sOpp) return -100;
        if (Board.sField[7][7] == sOpp) return -100;
        return 175;
    }
    if (sX == 6 && sY == 7) {
        if (Board.sField[7][7] == sOwn) return 175;
        if (Board.sField[7][7] == sOpp) return 2;
        if (Board.sField[5][7] == EMPTY) return -150;
        if (Board.sField[5][7] == sOpp) return -100;
        if (Board.sField[4][7] == EMPTY) return -150;
        if (Board.sField[4][7] == sOpp) return -100;
        if (Board.sField[3][7] == EMPTY) {
            if (Board.sField[4][6] == sOpp) return 10;
            else return -150;
        }
        if (Board.sField[3][7] == sOpp) return -100;
        if (Board.sField[2][7] == EMPTY) return 10;
        if (Board.sField[2][7] == sOpp) return -100;
        if (Board.sField[1][7] == EMPTY) {
            if (Board.sField[2][6] == sOwn) return -225;
            else return 10;
        }
        if (Board.sField[1][7] == sOpp) return -100;
        if (Board.sField[0][7] == sOpp) return -100;
        return 175;
    }
    if (sX == 1 && sY == 0) {
        if (Board.sField[0][0] == sOwn) return 175;
        if (Board.sField[0][0] == sOpp) return 2;
        if (Board.sField[2][0] == EMPTY) return -150;
        if (Board.sField[2][0] == sOpp) return -100;
        if (Board.sField[3][0] == EMPTY) return -150;
        if (Board.sField[3][0] == sOpp) return -100;
        if (Board.sField[4][0] == EMPTY) {
            if (Board.sField[3][1] == sOpp) return 10;
            else return -150;
        }
        if (Board.sField[4][0] == sOpp) return -100;
        if (Board.sField[5][0] == EMPTY) return 10;
        if (Board.sField[5][0] == sOpp) return -100;
        if (Board.sField[6][0] == EMPTY) {
            if (Board.sField[5][1] == sOwn) return -225;
            else return 10;
        }
        if (Board.sField[6][0] == sOpp) return -100;
        if (Board.sField[7][0] == sOpp) return -100;
        return 175;
    }
    if (sX == 6 && sY == 0) {
        if (Board.sField[7][0] == sOwn) return 175;
        if (Board.sField[7][0] == sOpp) return 2;
        if (Board.sField[5][0] == EMPTY) return -150;
        if (Board.sField[5][0] == sOpp) return -100;
        if (Board.sField[4][0] == EMPTY) return -150;
        if (Board.sField[4][0] == sOpp) return -100;
        if (Board.sField[3][0] == EMPTY) {
            if (Board.sField[4][1] == sOpp) return 10;
            else return -150;
        }
        if (Board.sField[3][0] == sOpp) return -100;
        if (Board.sField[2][0] == EMPTY) return 10;
        if (Board.sField[2][0] == sOpp) return -100;
        if (Board.sField[1][0] == EMPTY) {
            if (Board.sField[2][1] == sOwn) return -225;
            else return 10;
        }
        if (Board.sField[1][0] == sOpp) return -100;
        if (Board.sField[0][0] == sOpp) return -100;
        return 175;
    }
    if (sX == 7 && sY == 6) {
        if (Board.sField[7][7] == sOwn) return 175;
        if (Board.sField[7][7] == sOpp) return 2;
        if (Board.sField[7][5] == EMPTY) return -150;
        if (Board.sField[7][5] == sOpp) return -100;
        if (Board.sField[7][4] == EMPTY) return -150;
        if (Board.sField[7][4] == sOpp) return -100;
        if (Board.sField[7][3] == EMPTY) {
            if (Board.sField[6][4] == sOpp) return 10;
            else return -150;
        }
        if (Board.sField[7][3] == sOpp) return -100;
        if (Board.sField[7][2] == EMPTY) return 10;
        if (Board.sField[7][2] == sOpp) return -100;
        if (Board.sField[7][1] == EMPTY) {
            if (Board.sField[6][2] == sOwn) return -225;
            else return 10;
        }
        if (Board.sField[7][1] == sOpp) return -100;
        if (Board.sField[7][0] == sOpp) return -100;
        return 175;
    }
    if (sX == 7 && sY == 1) {
        if (Board.sField[7][0] == sOwn) return 175;
        if (Board.sField[7][0] == sOpp) return 2;
        if (Board.sField[7][2] == EMPTY) return -150;
        if (Board.sField[7][2] == sOpp) return -100;
        if (Board.sField[7][3] == EMPTY) return -150;
        if (Board.sField[7][3] == sOpp) return -100;
        if (Board.sField[7][4] == EMPTY) {
            if (Board.sField[6][3] == sOpp) return 10;
            else return -150;
        }
        if (Board.sField[7][4] == sOpp) return -100;
        if (Board.sField[7][5] == EMPTY) return 10;
        if (Board.sField[7][5] == sOpp) return -100;
        if (Board.sField[7][6] == EMPTY) {
            if (Board.sField[6][5] == sOwn) return -225;
            else return 10;
        }
        if (Board.sField[7][6] == sOpp) return -100;
        if (Board.sField[7][7] == sOpp) return -100;
        return 175;
    }
    if (sX == 0 && sY == 6) {
        if (Board.sField[0][7] == sOwn) return 175;
        if (Board.sField[0][7] == sOpp) return 2;
        if (Board.sField[0][5] == EMPTY) return -150;
        if (Board.sField[0][5] == sOpp) return -100;
        if (Board.sField[0][4] == EMPTY) return -150;
        if (Board.sField[0][4] == sOpp) return -100;
        if (Board.sField[0][3] == EMPTY) {
            if (Board.sField[1][4] == sOpp) return 10;
            else return -150;
        }
        if (Board.sField[0][3] == sOpp) return -100;
        if (Board.sField[0][2] == EMPTY) return 10;
        if (Board.sField[0][2] == sOpp) return -100;
        if (Board.sField[0][1] == EMPTY) {
            if (Board.sField[1][2] == sOwn) return -225;
            else return 10;
        }
        if (Board.sField[0][1] == sOpp) return -100;
        if (Board.sField[0][0] == sOpp) return -100;
        return 175;
    }
    if (sX == 0 && sY == 1) {
        if (Board.sField[0][0] == sOwn) return 175;
        if (Board.sField[0][0] == sOpp) return 2;
        if (Board.sField[0][2] == EMPTY) return -150;
        if (Board.sField[0][2] == sOpp) return -100;
        if (Board.sField[0][3] == EMPTY) return -150;
        if (Board.sField[0][3] == sOpp) return -100;
        if (Board.sField[0][4] == EMPTY) {
            if (Board.sField[1][3] == sOpp) return 10;
            else return -150;
        }
        if (Board.sField[0][4] == sOpp) return -100;
        if (Board.sField[0][5] == EMPTY) return 10;
        if (Board.sField[0][5] == sOpp) return -100;
        if (Board.sField[0][6] == EMPTY) {
            if (Board.sField[1][5] == sOwn) return -225;
            else return 10;
        }
        if (Board.sField[0][6] == sOpp) return -100;
        if (Board.sField[0][7] == sOpp) return -100;
        return 175;
    }

    /* A squares */
    if (sX == 2 && sY == 7) {
        if (Board.sField[3][7] != EMPTY) { if (Board.sField[2][6] == EMPTY) return 35; }
        else { if (Board.sField[4][7] == sOpp) return 25; else if (Board.sField[4][7] == EMPTY && Board.sField[5][7] == sOwn) return 35; }
        return 12;
    }
    if (sX == 5 && sY == 7) {
        if (Board.sField[4][7] != EMPTY) { if (Board.sField[5][6] == EMPTY) return 35; }
        else { if (Board.sField[3][7] == sOpp) return 25; else if (Board.sField[3][7] == EMPTY && Board.sField[2][7] == sOwn) return 35; }
        return 12;
    }
    if (sX == 7 && sY == 5) {
        if (Board.sField[7][4] != EMPTY) { if (Board.sField[6][3] == EMPTY) return 35; }
        else { if (Board.sField[7][3] == sOpp) return 25; else if (Board.sField[7][3] == EMPTY && Board.sField[7][2] == sOwn) return 35; }
        return 12;
    }
    if (sX == 7 && sY == 2) {
        if (Board.sField[7][3] != EMPTY) { if (Board.sField[6][2] == EMPTY) return 35; }
        else { if (Board.sField[7][4] == sOpp) return 25; else if (Board.sField[7][4] == EMPTY && Board.sField[7][5] == sOwn) return 35; }
        return 12;
    }
    if (sX == 5 && sY == 0) {
        if (Board.sField[4][0] != EMPTY) { if (Board.sField[5][1] == EMPTY) return 35; }
        else { if (Board.sField[3][0] == sOpp) return 25; else if (Board.sField[3][0] == EMPTY && Board.sField[2][0] == sOwn) return 35; }
        return 12;
    }
    if (sX == 2 && sY == 0) {
        if (Board.sField[3][0] != EMPTY) { if (Board.sField[2][1] == EMPTY) return 35; }
        else { if (Board.sField[4][0] == sOpp) return 25; else if (Board.sField[4][0] == EMPTY && Board.sField[5][0] == sOwn) return 35; }
        return 12;
    }
    if (sX == 0 && sY == 2) {
        if (Board.sField[0][3] != EMPTY) { if (Board.sField[1][2] == EMPTY) return 35; }
        else { if (Board.sField[0][4] == sOpp) return 25; else if (Board.sField[0][4] == EMPTY && Board.sField[0][5] == sOwn) return 35; }
        return 12;
    }
    if (sX == 0 && sY == 5) {
        if (Board.sField[0][4] != EMPTY) { if (Board.sField[1][5] == EMPTY) return 35; }
        else { if (Board.sField[0][3] == sOpp) return 25; else if (Board.sField[0][3] == EMPTY && Board.sField[0][2] == sOwn) return 35; }
        return 12;
    }

    /* B squares */
    if (sX == 3 && sY == 7) {
        if (Board.sField[4][7] == EMPTY) { if (Board.sField[5][7] == sOpp) return 30; }
        else { if (Board.sField[4][7] == Board.sField[2][7]) return 30; }
        return 12;
    }
    if (sX == 4 && sY == 7) {
        if (Board.sField[3][7] == EMPTY) { if (Board.sField[2][7] == sOpp) return 30; }
        else { if (Board.sField[3][7] == Board.sField[5][7]) return 30; }
        return 12;
    }
    if (sX == 7 && sY == 4) {
        if (Board.sField[7][3] == EMPTY) { if (Board.sField[7][2] == sOpp) return 30; }
        else { if (Board.sField[7][3] == Board.sField[7][5]) return 30; }
        return 12;
    }
    if (sX == 7 && sY == 3) {
        if (Board.sField[7][4] == EMPTY) { if (Board.sField[7][5] == sOpp) return 30; }
        else { if (Board.sField[7][4] == Board.sField[7][2]) return 30; }
        return 12;
    }
    if (sX == 4 && sY == 0) {
        if (Board.sField[3][0] == EMPTY) { if (Board.sField[2][0] == sOpp) return 30; }
        else { if (Board.sField[3][0] == Board.sField[5][0]) return 30; }
        return 12;
    }
    if (sX == 3 && sY == 0) {
        if (Board.sField[4][0] == EMPTY) { if (Board.sField[5][0] == sOpp) return 30; }
        else { if (Board.sField[4][0] == Board.sField[2][0]) return 30; }
        return 12;
    }
    if (sX == 0 && sY == 3) {
        if (Board.sField[0][4] == EMPTY) { if (Board.sField[0][5] == sOpp) return 30; }
        else { if (Board.sField[0][4] == Board.sField[0][2]) return 30; }
        return 12;
    }
    if (sX == 0 && sY == 4) {
        if (Board.sField[0][3] == EMPTY) { if (Board.sField[0][2] == sOpp) return 30; }
        else { if (Board.sField[0][5] == Board.sField[0][3]) return 30; }
        return 12;
    }

    /* X squares */
    if (sX == 1 && sY == 6) {
        if (Board.sField[0][7] == sOwn) return 100;
        if (Board.sField[0][7] == sOpp) return 2;
        if (Board.sField[6][7] == sOpp || Board.sField[0][1] == sOpp) return -100;
        return -200;
    }
    if (sX == 6 && sY == 6) {
        if (Board.sField[7][7] == sOwn) return 100;
        if (Board.sField[7][7] == sOpp) return 2;
        if (Board.sField[1][7] == sOpp || Board.sField[7][1] == sOpp) return -100;
        return -200;
    }
    if (sX == 6 && sY == 1) {
        if (Board.sField[7][0] == sOwn) return 100;
        if (Board.sField[7][0] == sOpp) return 2;
        if (Board.sField[7][6] == sOpp || Board.sField[1][0] == sOpp) return -100;
        return -200;
    }
    if (sX == 1 && sY == 1) {
        if (Board.sField[0][0] == sOwn) return 100;
        if (Board.sField[0][0] == sOpp) return 2;
        if (Board.sField[6][0] == sOpp || Board.sField[0][6] == sOpp) return -100;
        return -200;
    }

    /* F squares */
    if (sX == 2 && sY == 6) {
        if (Board.sField[2][7] != EMPTY && Board.sField[1][7] != EMPTY) return 25;
        if (Board.sField[3][7] == sOpp) return 5;
        return -5;
    }
    if (sX == 1 && sY == 5) {
        if (Board.sField[0][6] != EMPTY && Board.sField[0][5] != EMPTY) return 25;
        if (Board.sField[0][4] == sOpp) return 5;
        return -5;
    }
    if (sX == 1 && sY == 2) {
        if (Board.sField[0][1] != EMPTY && Board.sField[0][2] != EMPTY) return 25;
        if (Board.sField[0][3] == sOpp) return 5;
        return -5;
    }
    if (sX == 2 && sY == 1) {
        if (Board.sField[1][0] != EMPTY && Board.sField[2][0] != EMPTY) return 25;
        if (Board.sField[3][0] == sOpp) return 5;
        return -5;
    }
    if (sX == 5 && sY == 1) {
        if (Board.sField[6][0] != EMPTY && Board.sField[5][0] != EMPTY) return 25;
        if (Board.sField[4][0] == sOpp) return 5;
        return -5;
    }
    if (sX == 6 && sY == 2) {
        if (Board.sField[7][1] != EMPTY && Board.sField[7][2] != EMPTY) return 25;
        if (Board.sField[7][3] == sOpp) return 5;
        return -5;
    }
    if (sX == 6 && sY == 5) {
        if (Board.sField[7][6] != EMPTY && Board.sField[7][5] != EMPTY) return 25;
        if (Board.sField[7][4] == sOpp) return 5;
        return -5;
    }
    if (sX == 5 && sY == 6) {
        if (Board.sField[6][7] != EMPTY && Board.sField[5][7] != EMPTY) return 25;
        if (Board.sField[4][7] == sOpp) return 5;
        return -5;
    }

    /* G squares */
    if (sX == 3 && sY == 6) {
        if (Board.sField[2][6] != EMPTY || Board.sField[4][6] != EMPTY) return 30;
        return 18;
    }
    if (sX == 4 && sY == 6) {
        if (Board.sField[3][6] != EMPTY || Board.sField[5][6] != EMPTY) return 30;
        return 18;
    }
    if (sX == 6 && sY == 4) {
        if (Board.sField[6][3] != EMPTY || Board.sField[6][5] != EMPTY) return 30;
        return 18;
    }
    if (sX == 6 && sY == 3) {
        if (Board.sField[6][2] != EMPTY || Board.sField[6][4] != EMPTY) return 30;
        return 18;
    }
    if (sX == 4 && sY == 1) {
        if (Board.sField[5][1] != EMPTY || Board.sField[3][1] != EMPTY) return 30;
        return 18;
    }
    if (sX == 3 && sY == 1) {
        if (Board.sField[2][1] != EMPTY || Board.sField[4][1] != EMPTY) return 30;
        return 18;
    }
    if (sX == 1 && sY == 3) {
        if (Board.sField[1][2] != EMPTY || Board.sField[1][4] != EMPTY) return 30;
        return 18;
    }
    if (sX == 1 && sY == 4) {
        if (Board.sField[1][3] != EMPTY || Board.sField[1][5] != EMPTY) return 30;
        return 18;
    }

    /* D squares */
    if (sX == 2 && sY == 5) {
        if (Board.sField[3][5] != EMPTY && Board.sField[2][4] != EMPTY) return 50;
        return 20;
    }
    if (sX == 5 && sY == 5) {
        if (Board.sField[5][4] != EMPTY && Board.sField[4][5] != EMPTY) return 50;
        return 20;
    }
    if (sX == 5 && sY == 2) {
        if (Board.sField[5][3] != EMPTY && Board.sField[4][2] != EMPTY) return 50;
        return 20;
    }
    if (sX == 2 && sY == 2) {
        if (Board.sField[3][2] != EMPTY && Board.sField[2][3] != EMPTY) return 50;
        return 20;
    }

    /* E squares */
    if (sX == 3 && sY == 5) {
        if (Board.sField[2][5] != EMPTY || Board.sField[3][6] != EMPTY) return 40;
        return 20;
    }
    if (sX == 4 && sY == 5) {
        if (Board.sField[4][6] != EMPTY || Board.sField[5][5] != EMPTY) return 40;
        return 20;
    }
    if (sX == 5 && sY == 4) {
        if (Board.sField[6][4] != EMPTY || Board.sField[5][5] != EMPTY) return 40;
        return 20;
    }
    if (sX == 5 && sY == 3) {
        if (Board.sField[5][2] != EMPTY || Board.sField[6][3] != EMPTY) return 40;
        return 20;
    }
    if (sX == 4 && sY == 2) {
        if (Board.sField[5][2] != EMPTY || Board.sField[4][1] != EMPTY) return 40;
        return 20;
    }
    if (sX == 3 && sY == 2) {
        if (Board.sField[3][1] != EMPTY || Board.sField[2][2] != EMPTY) return 40;
        return 20;
    }
    if (sX == 2 && sY == 3) {
        if (Board.sField[2][2] != EMPTY || Board.sField[1][3] != EMPTY) return 40;
        return 20;
    }
    if (sX == 2 && sY == 4) {
        if (Board.sField[1][4] != EMPTY || Board.sField[2][5] != EMPTY) return 40;
        return 20;
    }

    return 0;
}

short sFlipped(PBOARD pBoard, short sX, short sY, short sWho)
{
    if ((sX == 0) && (sY == 2)) {
        if ((pBoard->sField[0][0] == sWho) && (pBoard->sField[0][1] == sWho)) return 150;
        else return 2;
    }
    if ((sX == 2) && (sY == 0)) {
        if ((pBoard->sField[0][0] == sWho) && (pBoard->sField[1][0] == sWho)) return 150;
        else return 2;
    }
    if ((sX == 5) && (sY == 0)) {
        if ((pBoard->sField[7][0] == sWho) && (pBoard->sField[6][0] == sWho)) return 150;
        else return 2;
    }
    if ((sX == 7) && (sY == 2)) {
        if ((pBoard->sField[7][0] == sWho) && (pBoard->sField[7][1] == sWho)) return 150;
        else return 2;
    }
    if ((sX == 0) && (sY == 5)) {
        if ((pBoard->sField[0][7] == sWho) && (pBoard->sField[0][6] == sWho)) return 150;
        else return 2;
    }
    if ((sX == 2) && (sY == 7)) {
        if ((pBoard->sField[0][7] == sWho) && (pBoard->sField[1][7] == sWho)) return 150;
        else return 2;
    }
    if ((sX == 5) && (sY == 7)) {
        if ((pBoard->sField[7][7] == sWho) && (pBoard->sField[6][7] == sWho)) return 150;
        else return 2;
    }
    if ((sX == 7) && (sY == 5)) {
        if ((pBoard->sField[7][7] == sWho) && (pBoard->sField[7][6] == sWho)) return 150;
        else return 2;
    }
    return (asFlipped[sX][sY]);
}

void CopyBoard(PBOARD pBoardFrom, PBOARD pBoardTo)
{
    short sX, sY;
    for(sX = 0; sX < 8; sX++)
        for(sY = 0; sY < 8; sY++)
            pBoardTo->sField[sX][sY] = pBoardFrom->sField[sX][sY];
}

BOOL fMustPass(BOARD Board, short sWho)
{
    short sX, sY;
    for(sX = 0; sX < 8; sX++)
        for(sY = 0; sY < 8; sY++)
            if (fIsMovePossible(Board, sX, sY, sWho)) return FALSE;
    return TRUE;
}

BOOL fGameOver(BOARD Board)
{
    return fMustPass(Board, COMPUTER) && fMustPass(Board, PLAYER);
}

BOOL fIsMovePossible(BOARD Board, short sX, short sY, short sWho)
{
    short i, sOpposite;
    if (Board.sField[sX][sY] != EMPTY) return FALSE;
    sOpposite = (sWho == PLAYER) ? COMPUTER : PLAYER;

    if (sY < 6) {
        i = 1;
        while ((sY + i) < 7 && Board.sField[sX][sY + i] == sOpposite) i++;
        if(Board.sField[sX][sY + i] == sWho && i > 1) return TRUE;
    }
    if (sY > 1) {
        i = 1;
        while ((sY - i) > 0 && Board.sField[sX][sY - i] == sOpposite) i++;
        if(Board.sField[sX][sY - i] == sWho && i > 1) return TRUE;
    }
    if (sX > 1) {
        i = 1;
        while ((sX - i) > 0 && Board.sField[sX - i][sY] == sOpposite) i++;
        if(Board.sField[sX - i][sY] == sWho && i > 1) return TRUE;
    }
    if (sX < 6) {
        i = 1;
        while ((sX + i) < 7 && Board.sField[sX + i][sY] == sOpposite) i++;
        if(Board.sField[sX + i][sY] == sWho && i > 1) return TRUE;
    }
    if ((sX < 6) && (sY < 6)) {
        i = 1;
        while ((sX + i) < 7 && (sY + i) < 7 && Board.sField[sX + i][sY + i] == sOpposite) i++;
        if(Board.sField[sX + i][sY + i] == sWho && i > 1) return TRUE;
    }
    if ((sX < 6) && (sY > 1)) {
        i = 1;
        while ((sX + i) < 7 && (sY - i) > 0 && Board.sField[sX + i][sY - i] == sOpposite) i++;
        if(Board.sField[sX + i][sY - i] == sWho && i > 1) return TRUE;
    }
    if ((sX > 1) && (sY > 1)) {
        i = 1;
        while ((sX - i) > 0 && (sY - i) > 0 && Board.sField[sX - i][sY - i] == sOpposite) i++;
        if(Board.sField[sX - i][sY - i] == sWho && i > 1) return TRUE;
    }
    if ((sX > 1) && (sY < 6)) {
        i = 1;
        while ((sY + i) < 7 && (sX - i) > 0 && Board.sField[sX - i][sY + i] == sOpposite) i++;
        if(Board.sField[sX - i][sY + i] == sWho && i > 1) return TRUE;
    }
    return FALSE;
}

short MakeMove(PBOARD pBoard, short sX, short sY, short sWho, BOOL fSimple, BOOL fInvalidate, short cX, short cY, HWND hwnd)
{
    short i, sOpposite, sNumberOfDirections = 0, sDiscsFlipped = 0, sDirectionsEval;
    BOARD boardHilf;
    RECT rect;
    if (!fSimple) CopyBoard(pBoard, &boardHilf);
    sOpposite = (sWho == PLAYER) ? COMPUTER : PLAYER;
    pBoard->sField[sX][sY] = sWho;
    if (fInvalidate) {
        rect.left = (sX+1)*cX; rect.right = (sX+2)*cX;
        rect.top = (sY+2)*cY; rect.bottom = (sY+1)*cY;
        InvalidateRect(hwnd, &rect, FALSE);
    }

    /* Север */
    if (sY < 6) {
        i = 1;
        while ((sY + i) < 7 && pBoard->sField[sX][sY + i] == sOpposite) i++;
        if(pBoard->sField[sX][sY + i] == sWho && i > 1) {
            sNumberOfDirections++;
            while(i > 1) {
                i--;
                pBoard->sField[sX][sY + i] = sWho;
                if (fInvalidate) {
                    rect.left = (sX+1)*cX; rect.right = (sX+2)*cX;
                    rect.top = (sY + i + 2)*cY; rect.bottom = (sY + i + 1)*cY;
                    InvalidateRect(hwnd, &rect, FALSE);
                }
                if (!fSimple) sDiscsFlipped += sFlipped(&boardHilf, sX, sY + i, sWho);
            }
        }
    }
    /* Юг */
    if (sY > 1) {
        i = 1;
        while ((sY - i) > 0 && pBoard->sField[sX][sY - i] == sOpposite) i++;
        if(pBoard->sField[sX][sY - i] == sWho && i > 1) {
            sNumberOfDirections++;
            while(i > 1) {
                i--;
                pBoard->sField[sX][sY - i] = sWho;
                if (fInvalidate) {
                    rect.left = (sX+1)*cX; rect.right = (sX+2)*cX;
                    rect.top = (sY - i + 2)*cY; rect.bottom = (sY - i + 1)*cY;
                    InvalidateRect(hwnd, &rect, FALSE);
                }
                if (!fSimple) sDiscsFlipped += sFlipped(&boardHilf, sX, sY - i, sWho);
            }
        }
    }
    /* Запад */
    if (sX > 1) {
        i = 1;
        while ((sX - i) > 0 && pBoard->sField[sX - i][sY] == sOpposite) i++;
        if(pBoard->sField[sX - i][sY] == sWho && i > 1) {
            sNumberOfDirections++;
            while(i > 1) {
                i--;
                pBoard->sField[sX - i][sY] = sWho;
                if (fInvalidate) {
                    rect.left = (sX - i + 1)*cX; rect.right = (sX - i + 2)*cX;
                    rect.top = (sY + 2)*cY; rect.bottom = (sY + 1)*cY;
                    InvalidateRect(hwnd, &rect, FALSE);
                }
                if (!fSimple) sDiscsFlipped += sFlipped(&boardHilf, sX - i, sY, sWho);
            }
        }
    }
    /* Восток */
    if (sX < 6) {
        i = 1;
        while ((sX + i) < 7 && pBoard->sField[sX + i][sY] == sOpposite) i++;
        if(pBoard->sField[sX + i][sY] == sWho && i > 1) {
            sNumberOfDirections++;
            while(i > 1) {
                i--;
                pBoard->sField[sX + i][sY] = sWho;
                if (fInvalidate) {
                    rect.left = (sX + i + 1)*cX; rect.right = (sX + i + 2)*cX;
                    rect.top = (sY + 2)*cY; rect.bottom = (sY + 1)*cY;
                    InvalidateRect(hwnd, &rect, FALSE);
                }
                if (!fSimple) sDiscsFlipped += sFlipped(&boardHilf, sX + i, sY, sWho);
            }
        }
    }
    /* Северо-восток */
    if ((sX < 6) && (sY < 6)) {
        i = 1;
        while ((sX + i) < 7 && (sY + i) < 7 && pBoard->sField[sX + i][sY + i] == sOpposite) i++;
        if(pBoard->sField[sX + i][sY + i] == sWho && i > 1) {
            sNumberOfDirections++;
            while(i > 1) {
                i--;
                pBoard->sField[sX + i][sY + i] = sWho;
                if (fInvalidate) {
                    rect.left = (sX + i + 1)*cX; rect.right = (sX + i + 2)*cX;
                    rect.top = (sY + i + 2)*cY; rect.bottom = (sY + i + 1)*cY;
                    InvalidateRect(hwnd, &rect, FALSE);
                }
                if (!fSimple) sDiscsFlipped += sFlipped(&boardHilf, sX + i, sY + i, sWho);
            }
        }
    }
    /* Юго-восток */
    if ((sX < 6) && (sY > 1)) {
        i = 1;
        while ((sX + i) < 7 && (sY - i) > 0 && pBoard->sField[sX + i][sY - i] == sOpposite) i++;
        if(pBoard->sField[sX + i][sY - i] == sWho && i > 1) {
            sNumberOfDirections++;
            while(i > 1) {
                i--;
                pBoard->sField[sX + i][sY - i] = sWho;
                if (fInvalidate) {
                    rect.left = (sX + i + 1)*cX; rect.right = (sX + i + 2)*cX;
                    rect.top = (sY - i + 2)*cY; rect.bottom = (sY - i + 1)*cY;
                    InvalidateRect(hwnd, &rect, FALSE);
                }
                if (!fSimple) sDiscsFlipped += sFlipped(&boardHilf, sX + i, sY - i, sWho);
            }
        }
    }
    /* Юго-запад */
    if ((sX > 1) && (sY > 1)) {
        i = 1;
        while ((sX - i) > 0 && (sY - i) > 0 && pBoard->sField[sX - i][sY - i] == sOpposite) i++;
        if(pBoard->sField[sX - i][sY - i] == sWho && i > 1) {
            sNumberOfDirections++;
            while(i > 1) {
                i--;
                pBoard->sField[sX - i][sY - i] = sWho;
                if (fInvalidate) {
                    rect.left = (sX - i + 1)*cX; rect.right = (sX - i + 2)*cX;
                    rect.top = (sY - i + 2)*cY; rect.bottom = (sY - i + 1)*cY;
                    InvalidateRect(hwnd, &rect, FALSE);
                }
                if (!fSimple) sDiscsFlipped += sFlipped(&boardHilf, sX - i, sY - i, sWho);
            }
        }
    }
    /* Северо-запад */
    if ((sX > 1) && (sY < 6)) {
        i = 1;
        while ((sY + i) < 7 && (sX - i) > 0 && pBoard->sField[sX - i][sY + i] == sOpposite) i++;
        if(pBoard->sField[sX - i][sY + i] == sWho && i > 1) {
            sNumberOfDirections++;
            while(i > 1) {
                i--;
                pBoard->sField[sX - i][sY + i] = sWho;
                if (fInvalidate) {
                    rect.left = (sX - i + 1)*cX; rect.right = (sX - i + 2)*cX;
                    rect.top = (sY + i + 2)*cY; rect.bottom = (sY + i + 1)*cY;
                    InvalidateRect(hwnd, &rect, FALSE);
                }
                if (!fSimple) sDiscsFlipped += sFlipped(&boardHilf, sX - i, sY + i, sWho);
            }
        }
    }

    if (fSimple) return 0;

    switch (sNumberOfDirections) {
        case 1: sDirectionsEval = 0; break;
        case 2: sDirectionsEval = 5; break;
        case 3: sDirectionsEval = 15; break;
        default: sDirectionsEval = 20; break;
    }
    if (sWho == COMPUTER)
        return (sDiscsFlipped - sDirectionsEval);
    else
        return (sDirectionsEval - sDiscsFlipped);
}

short sBewertung(BOARD Board, short sXMoved, short sYMoved, short sWho)
{
    short sMobility = 0, sX, sY, sOptions = 0;
    for(sX=0; sX<8; sX++)
        for(sY=0; sY<8; sY++) {
            if (fIsMovePossible(Board, sX, sY, sWho)) {
                sOptions++;
                if ((sX==1)&&(sY==1)) { if(Board.sField[0][0]!=EMPTY) sMobility+=40; continue; }
                if ((sX==1)&&(sY==6)) { if(Board.sField[0][7]!=EMPTY) sMobility+=40; continue; }
                if ((sX==6)&&(sY==1)) { if(Board.sField[7][0]!=EMPTY) sMobility+=40; continue; }
                if ((sX==6)&&(sY==6)) { if(Board.sField[7][7]!=EMPTY) sMobility+=40; continue; }
                if ((sX==1)&&(sY==0)&&(Board.sField[0][0]==EMPTY)) { sMobility+=20; continue; }
                if ((sX==0)&&(sY==1)&&(Board.sField[0][0]==EMPTY)) { sMobility+=20; continue; }
                if ((sX==0)&&(sY==6)&&(Board.sField[0][7]==EMPTY)) { sMobility+=20; continue; }
                if ((sX==1)&&(sY==7)&&(Board.sField[0][7]==EMPTY)) { sMobility+=20; continue; }
                if ((sX==6)&&(sY==7)&&(Board.sField[7][7]==EMPTY)) { sMobility+=20; continue; }
                if ((sX==7)&&(sY==6)&&(Board.sField[7][7]==EMPTY)) { sMobility+=20; continue; }
                if ((sX==6)&&(sY==0)&&(Board.sField[7][0]==EMPTY)) { sMobility+=20; continue; }
                if ((sX==7)&&(sY==1)&&(Board.sField[7][0]==EMPTY)) { sMobility+=20; continue; }
                sMobility += 40;
            }
        }
    if (sOptions == 1) sMobility = 0;
    if (sWho == COMPUTER)
        return (sMobility + sSquare(Board, COMPUTER, PLAYER, sXMoved, sYMoved));
    else
        return (-(sMobility + sSquare(Board, PLAYER, COMPUTER, sXMoved, sYMoved)));
}

short sOthello(BOARD Board, short sX, short sY, short sCurrentLevel, short sMaxLevel, short sWho, short *psX, short *psY, BOOL *pfValid, short sPrevBewertung)
{
    BOOL fGotAMove, fTemp;
    short max = -IND, min = IND, sXInd, sYInd, sWhoNext, sMaxIndizesInd, sTemp1, sTemp2, sBew, sDirectionsComp;
    short sMaxIndizes[64][2];

    if (sCurrentLevel == 0) {
        sMaxIndizesInd = 0;
        fGotAMove = FALSE;
        for (sXInd = 0; sXInd < 8; sXInd++)
            for (sYInd = 0; sYInd < 8; sYInd++) {
                if (fIsMovePossible(Board, sXInd, sYInd, COMPUTER)) {
                    fGotAMove = TRUE;
                    sBew = sOthello(Board, sXInd, sYInd, sCurrentLevel+1, sMaxLevel, sWho, &sTemp1, &sTemp2, &fTemp, 0);
                    if(sBew > max) {
                        max = sBew; sMaxIndizesInd = 0;
                        sMaxIndizes[0][0] = sXInd; sMaxIndizes[0][1] = sYInd;
                    } else if(sBew == max) {
                        sMaxIndizesInd++;
                        sMaxIndizes[sMaxIndizesInd][0] = sXInd;
                        sMaxIndizes[sMaxIndizesInd][1] = sYInd;
                    }
                }
            }
        if(fGotAMove) {
            sBew = abs(rand() % (sMaxIndizesInd + 1));
            *psX = sMaxIndizes[sBew][0];
            *psY = sMaxIndizes[sBew][1];
            *pfValid = TRUE;
        } else *pfValid = FALSE;
        return 0;
    } else {
        sDirectionsComp = MakeMove(&Board, sX, sY, sWho, FALSE, FALSE, 0, 0, NULL);
        if (sCurrentLevel == sMaxLevel)
            return (sBewertung(Board, sX, sY, sWho) + sPrevBewertung + sDirectionsComp);
        sWhoNext = (sWho == COMPUTER) ? PLAYER : COMPUTER;
        for (sXInd = 0; sXInd < 8; sXInd++)
            for (sYInd = 0; sYInd < 8; sYInd++) {
                if (fIsMovePossible(Board, sXInd, sYInd, sWhoNext)) {
                    sBew = sOthello(Board, sXInd, sYInd, sCurrentLevel+1, sMaxLevel, sWhoNext, &sTemp1, &sTemp2, &fTemp, sPrevBewertung + sBewertung(Board, sX, sY, sWho) + sDirectionsComp);
                    if (sBew <= min) min = sBew;
                }
            }
        return min;
    }
}
