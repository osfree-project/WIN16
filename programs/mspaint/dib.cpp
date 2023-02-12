/*
 * PROJECT:     PAINT for ReactOS
 * LICENSE:     LGPL
 * FILE:        base/applications/mspaint/dib.cpp
 * PURPOSE:     Some DIB related functions
 * PROGRAMMERS: Benedikt Freisen
 */

/* INCLUDES *********************************************************/

#include "precomp.h"
#include <math.h>

/* FUNCTIONS ********************************************************/

HBITMAP
CreateDIBWithProperties(int width, int height)
{
    BITMAPINFO bmi;
    ZeroMemory(&bmi, sizeof(BITMAPINFO));
    bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
    bmi.bmiHeader.biWidth = width;
    bmi.bmiHeader.biHeight = height;
    bmi.bmiHeader.biPlanes = 1;
    bmi.bmiHeader.biBitCount = 24;
    bmi.bmiHeader.biCompression = BI_RGB;
    return CreateDIBSection(NULL, &bmi, DIB_RGB_COLORS, NULL, NULL, 0);
}

HBITMAP
CreateColorDIB(int width, int height, COLORREF rgb)
{
    HBITMAP ret = CreateDIBWithProperties(width, height);
    if (!ret)
        return NULL;

    if (rgb)
    {
        HDC hdc = CreateCompatibleDC(NULL);
        HGDIOBJ hbmOld = SelectObject(hdc, ret);
        RECT rc;
        SetRect(&rc, 0, 0, width, height);
        HBRUSH hbr = CreateSolidBrush(rgb);
        FillRect(hdc, &rc, hbr);
        DeleteObject(hbr);
        SelectObject(hdc, hbmOld);
        DeleteDC(hdc);
    }

    return ret;
}

int
GetDIBWidth(HBITMAP hBitmap)
{
    BITMAP bm;
    GetObject(hBitmap, sizeof(BITMAP), &bm);
    return bm.bmWidth;
}

int
GetDIBHeight(HBITMAP hBitmap)
{
    BITMAP bm;
    GetObject(hBitmap, sizeof(BITMAP), &bm);
    return bm.bmHeight;
}

BOOL SaveDIBToFile(HBITMAP hBitmap, LPTSTR FileName, HDC hDC)
{
    CImage img;
    img.Attach(hBitmap);
    img.Save(FileName);  // TODO: error handling
    img.Detach();

    WIN32_FIND_DATA find;
    HANDLE hFind = FindFirstFile(FileName, &find);
    if (hFind == INVALID_HANDLE_VALUE)
    {
        ShowFileLoadError(FileName);
        return FALSE;
    }
    FindClose(hFind);

    // update time and size
    FILETIME ft;
    FileTimeToLocalFileTime(&find.ftLastWriteTime, &ft);
    FileTimeToSystemTime(&ft, &fileTime);
    fileSize = find.nFileSizeLow;

    // TODO: update hRes and vRes

    registrySettings.SetMostRecentFile(FileName);

    isAFile = TRUE;
    imageSaved = TRUE;
    return TRUE;
}

void ShowFileLoadError(LPCTSTR name)
{
    CString strText;
    strText.Format(IDS_LOADERRORTEXT, (LPCTSTR) name);
    CString strProgramName;
    strProgramName.LoadString(IDS_PROGRAMNAME);
    mainWindow.MessageBox(strText, strProgramName, MB_OK | MB_ICONEXCLAMATION);
}

HBITMAP SetBitmapAndInfo(HBITMAP hBitmap, LPCTSTR name, DWORD dwFileSize, BOOL isFile)
{
    if (hBitmap == NULL)
    {
        COLORREF white = RGB(255, 255, 255);
        hBitmap = CreateColorDIB(registrySettings.BMPWidth,
                                 registrySettings.BMPHeight, white);
        if (hBitmap == NULL)
            return FALSE;

        fileHPPM = fileVPPM = 2834;
        ZeroMemory(&fileTime, sizeof(fileTime));
        isFile = FALSE;
    }
    else
    {
        // update PPMs
        HDC hScreenDC = GetDC(NULL);
        fileHPPM = (int)(GetDeviceCaps(hScreenDC, LOGPIXELSX) * 1000 / 25.4);
        fileVPPM = (int)(GetDeviceCaps(hScreenDC, LOGPIXELSY) * 1000 / 25.4);
        ReleaseDC(NULL, hScreenDC);
    }

    // update image
    imageModel.Insert(hBitmap);
    imageModel.ClearHistory();

    // update fileSize
    fileSize = dwFileSize;

    // update filepathname
    if (name && name[0])
        GetFullPathName(name, _countof(filepathname), filepathname, NULL);
    else
        LoadString(hProgInstance, IDS_DEFAULTFILENAME, filepathname, _countof(filepathname));

    // set title
    CString strTitle;
    strTitle.Format(IDS_WINDOWTITLE, PathFindFileName(filepathname));
    mainWindow.SetWindowText(strTitle);

    // update file info and recent
    isAFile = isFile;
    if (isAFile)
        registrySettings.SetMostRecentFile(filepathname);

    imageSaved = TRUE;

    return hBitmap;
}

HBITMAP DoLoadImageFile(HWND hwnd, LPCTSTR name, BOOL fIsMainFile)
{
    // find the file
    WIN32_FIND_DATA find;
    HANDLE hFind = FindFirstFile(name, &find);
    if (hFind == INVALID_HANDLE_VALUE)
    {
        // does not exist
        CStringW strText;
        strText.Format(IDS_LOADERRORTEXT, name);
        MessageBoxW(hwnd, strText, NULL, MB_ICONERROR);
        return NULL;
    }
    DWORD dwFileSize = find.nFileSizeLow; // get file size
    FindClose(hFind);

    // is file empty?
    if (dwFileSize == 0)
    {
        if (fIsMainFile)
        {
            FILETIME ft;
            FileTimeToLocalFileTime(&find.ftLastWriteTime, &ft);
            FileTimeToSystemTime(&ft, &fileTime);
            return SetBitmapAndInfo(NULL, name, dwFileSize, TRUE);
        }
    }

    // load the image
    CImage img;
    img.Load(name);
    HBITMAP hBitmap = img.Detach();

    if (hBitmap == NULL)
    {
        // cannot open
        CStringW strText;
        strText.Format(IDS_LOADERRORTEXT, name);
        MessageBoxW(hwnd, strText, NULL, MB_ICONERROR);
        return NULL;
    }

    if (fIsMainFile)
    {
        FILETIME ft;
        FileTimeToLocalFileTime(&find.ftLastWriteTime, &ft);
        FileTimeToSystemTime(&ft, &fileTime);
        SetBitmapAndInfo(hBitmap, name, dwFileSize, TRUE);
    }

    return hBitmap;
}

HBITMAP Rotate90DegreeBlt(HDC hDC1, INT cx, INT cy, BOOL bRight)
{
    HBITMAP hbm2 = CreateDIBWithProperties(cy, cx);
    if (!hbm2)
        return NULL;

    HDC hDC2 = CreateCompatibleDC(NULL);
    HGDIOBJ hbm2Old = SelectObject(hDC2, hbm2);
    if (bRight)
    {
        for (INT y = 0; y < cy; ++y)
        {
            for (INT x = 0; x < cx; ++x)
            {
                COLORREF rgb = GetPixel(hDC1, x, y);
                SetPixelV(hDC2, cy - (y + 1), x, rgb);
            }
        }
    }
    else
    {
        for (INT y = 0; y < cy; ++y)
        {
            for (INT x = 0; x < cx; ++x)
            {
                COLORREF rgb = GetPixel(hDC1, x, y);
                SetPixelV(hDC2, y, cx - (x + 1), rgb);
            }
        }
    }
    SelectObject(hDC2, hbm2Old);
    DeleteDC(hDC2);
    return hbm2;
}

#ifndef M_PI
    #define M_PI 3.14159265
#endif

HBITMAP SkewDIB(HDC hDC1, HBITMAP hbm, INT nDegree, BOOL bVertical)
{
    if (nDegree == 0)
        return CopyDIBImage(hbm);

    const double eTan = tan(abs(nDegree) * M_PI / 180);

    BITMAP bm;
    GetObjectW(hbm, sizeof(bm), &bm);
    INT cx = bm.bmWidth, cy = bm.bmHeight, dx = 0, dy = 0;
    if (bVertical)
        dy = INT(cx * eTan);
    else
        dx = INT(cy * eTan);

    if (dx == 0 && dy == 0)
        return CopyDIBImage(hbm);

    HBITMAP hbmNew = CreateColorDIB(cx + dx, cy + dy, RGB(255, 255, 255));
    if (!hbmNew)
        return NULL;

    HDC hDC2 = CreateCompatibleDC(NULL);
    HGDIOBJ hbm2Old = SelectObject(hDC2, hbmNew);
    if (bVertical)
    {
        for (INT x = 0; x < cx; ++x)
        {
            INT delta = INT(x * eTan);
            if (nDegree > 0)
                BitBlt(hDC2, x, (dy - delta), 1, cy, hDC1, x, 0, SRCCOPY);
            else
                BitBlt(hDC2, x, delta, 1, cy, hDC1, x, 0, SRCCOPY);
        }
    }
    else
    {
        for (INT y = 0; y < cy; ++y)
        {
            INT delta = INT(y * eTan);
            if (nDegree > 0)
                BitBlt(hDC2, (dx - delta), y, cx, 1, hDC1, 0, y, SRCCOPY);
            else
                BitBlt(hDC2, delta, y, cx, 1, hDC1, 0, y, SRCCOPY);
        }
    }
    SelectObject(hDC2, hbm2Old);
    DeleteDC(hDC2);
    return hbmNew;
}
