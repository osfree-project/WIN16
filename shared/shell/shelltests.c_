/*
 * Copyright 2010 Piotr Caban for CodeWeavers
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
#include <wine/test.h>

#include "windows.h"
#include "shellapi.h"




#define WM_EXPECTED_VALUE WM_APP
#define DROP_NC_AREA       1
#define DROP_WIDE_FILENAME 2
struct DragParam {
    HWND hwnd;
    HANDLE ready;
};

static LRESULT WINAPI drop_window_proc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)
{
    static DWORD flags;
    static char expected_filename[MAX_PATH];

    switch (msg) {
    case WM_EXPECTED_VALUE:
    {
        lstrcpyn(expected_filename, (const char*)wparam, sizeof(expected_filename));
        flags = lparam;
        break;
    }
    case WM_DROPFILES:
    {
        HDROP hDrop = (HDROP)wparam;
        char filename[MAX_PATH] = "dummy";
        POINT pt;
        BOOL r;
        UINT num, len;
        winetest_push_context("%s(%lu)", wine_dbgstr_a(expected_filename), flags);

        num = DragQueryFile(hDrop, 0xffffffff, NULL, 0);
        ok(num == 1, "expected 1, got %u\n", num);

        num = DragQueryFile(hDrop, 0xffffffff, (char*)0xdeadbeef, 0xffffffff);
        ok(num == 1, "expected 1, got %u\n", num);

        len = strlen(expected_filename);
        num = DragQueryFile(hDrop, 0, NULL, 0);
        ok(num == len, "expected %u, got %u\n", len, num);

        num = DragQueryFile(hDrop, 0, filename, 0);
        ok(num == len, "expected %u, got %u\n", len, num);
        ok(!strcmp(filename, "dummy"), "got %s\n", filename);

        num = DragQueryFile(hDrop, 0, filename, sizeof(filename));
        ok(num == len, "expected %u, got %u\n", len, num);
        ok(!strcmp(filename, expected_filename), "expected %s, got %s\n",
           expected_filename, filename);

        memset(filename, 0xaa, sizeof(filename));
        num = DragQueryFile(hDrop, 0, filename, 2);
        ok(num == 1, "expected 1, got %u\n", num);
        ok(filename[0] == expected_filename[0], "expected '%c', got '%c'\n",
           expected_filename[0], filename[0]);
        ok(filename[1] == '\0', "expected nul, got %#x\n", (BYTE)filename[1]);

        r = DragQueryPoint(hDrop, &pt);
        ok(r == !(flags & DROP_NC_AREA), "expected %d, got %d\n", !(flags & DROP_NC_AREA), r);
        ok(pt.x == 10, "expected 10, got %ld\n", pt.x);
        ok(pt.y == 20, "expected 20, got %ld\n", pt.y);
        DragFinish(hDrop);
        winetest_pop_context();
        return 0;
    }
    }
    return DefWindowProc(hwnd, msg, wparam, lparam);
}

static DWORD WINAPI drop_window_therad(void *arg)
{
    struct DragParam *param = arg;
    WNDCLASS cls;
    WINDOWINFO info;
    BOOL r;
    MSG msg;

    memset(&cls, 0, sizeof(cls));
    cls.lpfnWndProc = drop_window_proc;
    cls.hInstance = GetModuleHandle(NULL);
    cls.lpszClassName = "drop test";
    RegisterClass(&cls);

    param->hwnd = CreateWindow("drop test", NULL, 0, 0, 0, 0, 0,
                                NULL, 0, NULL, 0);
    ok(param->hwnd != NULL, "CreateWindow failed: %ld\n", GetLastError());

    memset(&info, 0, sizeof(info));
    info.cbSize = sizeof(info);
    r = GetWindowInfo(param->hwnd, &info);
    ok(r, "got %d\n", r);
    ok(!(info.dwExStyle & WS_EX_ACCEPTFILES), "got %08lx\n", info.dwExStyle);

    DragAcceptFiles(param->hwnd, TRUE);

    memset(&info, 0, sizeof(info));
    info.cbSize = sizeof(info);
    r = GetWindowInfo(param->hwnd, &info);
    ok(r, "got %d\n", r);
    ok((info.dwExStyle & WS_EX_ACCEPTFILES), "got %08lx\n", info.dwExStyle);

    SetEvent(param->ready);

    while ((r = GetMessage(&msg, NULL, 0, 0)) != 0) {
        if (r == (BOOL)-1) {
            ok(0, "unexpected return value, got %d\n", r);
            break;
        }
        DispatchMessage(&msg);
    }

    DestroyWindow(param->hwnd);
    UnregisterClass("drop test", GetModuleHandle(NULL));
    return 0;
}

static void test_DragQueryFile(BOOL non_client_flag)
{
    struct DragParam param;
    HANDLE hThread;
    DWORD rc;
    HGLOBAL hDrop;
    DROPFILES *pDrop;
    int ret, i;
    BOOL r;
    static const struct {
        UINT codepage;
        const char* filename;
    } testcase[] = {
        { 0, "c:\\wintest.bin" },
        { 932, "d:\\\x89\xb9\x8a\x79_02.CHY" },
    };

    param.ready = CreateEvent(NULL, FALSE, FALSE, NULL);
    ok(param.ready != NULL, "can't create event\n");
    hThread = CreateThread(NULL, 0, drop_window_therad, &param, 0, NULL);

    rc = WaitForSingleObject(param.ready, 5000);
    ok(rc == WAIT_OBJECT_0, "got %lu\n", rc);

    for (i = 0; i < ARRAY_SIZE(testcase); i++)
    {
        winetest_push_context("%d", i);
        if (testcase[i].codepage && testcase[i].codepage != GetACP())
        {
            skip("need codepage %u for this test\n", testcase[i].codepage);
            winetest_pop_context();
            continue;
        }

        ret = MultiByteToWideChar(CP_ACP, 0, testcase[i].filename, -1, NULL, 0);
        ok(ret > 0, "got %d\n", ret);
        hDrop = GlobalAlloc(GHND, sizeof(DROPFILES) + (ret + 1) * sizeof(WCHAR));
        pDrop = GlobalLock(hDrop);
        pDrop->pt.x = 10;
        pDrop->pt.y = 20;
        pDrop->fNC = non_client_flag;
        pDrop->pFiles = sizeof(DROPFILES);
        ret = MultiByteToWideChar(CP_ACP, 0, testcase[i].filename, -1,
                                  (LPWSTR)(pDrop + 1), ret);
        ok(ret > 0, "got %d\n", ret);
        pDrop->fWide = TRUE;
        GlobalUnlock(hDrop);

        r = PostMessage(param.hwnd, WM_EXPECTED_VALUE,
                         (WPARAM)testcase[i].filename, DROP_WIDE_FILENAME | (non_client_flag ? DROP_NC_AREA : 0));
        ok(r, "got %d\n", r);

        r = PostMessage(param.hwnd, WM_DROPFILES, (WPARAM)hDrop, 0);
        ok(r, "got %d\n", r);

        hDrop = GlobalAlloc(GHND, sizeof(DROPFILES) + strlen(testcase[i].filename) + 2);
        pDrop = GlobalLock(hDrop);
        pDrop->pt.x = 10;
        pDrop->pt.y = 20;
        pDrop->fNC = non_client_flag;
        pDrop->pFiles = sizeof(DROPFILES);
        strcpy((char *)(pDrop + 1), testcase[i].filename);
        pDrop->fWide = FALSE;
        GlobalUnlock(hDrop);

        r = PostMessage(param.hwnd, WM_EXPECTED_VALUE,
                         (WPARAM)testcase[i].filename, non_client_flag ? DROP_NC_AREA : 0);
        ok(r, "got %d\n", r);

        r = PostMessage(param.hwnd, WM_DROPFILES, (WPARAM)hDrop, 0);
        ok(r, "got %d\n", r);

        winetest_pop_context();
    }

    r = PostMessage(param.hwnd, WM_QUIT, 0, 0);
    ok(r, "got %d\n", r);

    rc = WaitForSingleObject(hThread, 5000);
    ok(rc == WAIT_OBJECT_0, "got %ld\n", rc);

    CloseHandle(param.ready);
    CloseHandle(hThread);
}

START_TEST(shellole)
{

    test_DragQueryFile(TRUE);
    test_DragQueryFile(FALSE);

}
