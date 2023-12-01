/*
 * PROJECT:     PAINT for ReactOS
 * LICENSE:     LGPL
 * FILE:        base/applications/mspaint/registry.cpp
 * PURPOSE:     Offering functions dealing with registry values
 * PROGRAMMERS: Benedikt Freisen
 *              Katayama Hirofumi MZ
 */

/* INCLUDES *********************************************************/

#include "precomp.h"
#include <winreg.h>
#include <wincon.h>
#include <shlobj.h>

/* FUNCTIONS ********************************************************/
static DWORD ReadDWORD(CRegKey &key, LPCTSTR lpName, DWORD &dwValue, BOOL bCheckForDef)
{
    DWORD dwPrev = dwValue;

    if (key.QueryDWORDValue(lpName, dwValue) != ERROR_SUCCESS || (bCheckForDef && dwValue == 0))
        dwValue = dwPrev;

    return dwPrev;
}

static void ReadString(CRegKey &key, LPCTSTR lpName, CString &strValue, LPCTSTR lpDefault = TEXT(""))
{
    CString strTemp;
    ULONG nChars = MAX_PATH;
    LPTSTR psz = strTemp.GetBuffer(nChars);
    LONG error = key.QueryStringValue(lpName, psz, &nChars);
    strTemp.ReleaseBuffer();

    if (error == ERROR_SUCCESS)
        strValue = strTemp;
    else
        strValue = lpDefault;
}

void RegistrySettings::SetWallpaper(LPCTSTR szFileName, RegistrySettings::WallpaperStyle style)
{
    CRegKey desktop;
    if (desktop.Open(HKEY_CURRENT_USER, _T("Control Panel\\Desktop")) == ERROR_SUCCESS)
    {
        desktop.SetStringValue(_T("Wallpaper"), szFileName);

        desktop.SetStringValue(_T("WallpaperStyle"), (style == RegistrySettings::STRETCHED) ? _T("2") : _T("0"));
        desktop.SetStringValue(_T("TileWallpaper"), (style == RegistrySettings::TILED) ? _T("1") : _T("0"));
    }

    SystemParametersInfo(SPI_SETDESKWALLPAPER, 0, (PVOID) szFileName, SPIF_UPDATEINIFILE | SPIF_SENDCHANGE);
}

void RegistrySettings::LoadPresets()
{
    BMPHeight = GetSystemMetrics(SM_CYSCREEN) / 2;
    BMPWidth = GetSystemMetrics(SM_CXSCREEN) / 2;
    GridExtent = 1;
    NoStretching = 0;
    ShowThumbnail = 0;
    SnapToGrid = 0;
    ThumbHeight = 100;
    ThumbWidth = 120;
    ThumbXPos = 180;
    ThumbYPos = 200;
    UnitSetting = 0;
    Bold = FALSE;
    Italic = FALSE;
    Underline = FALSE;
    CharSet = DEFAULT_CHARSET;
    PointSize = 14;
    FontsPositionX = 0;
    FontsPositionY = 0;
    ShowTextTool = TRUE;

    LOGFONT lf;
    GetObject(GetStockObject(DEFAULT_GUI_FONT), sizeof(lf), &lf);
    strFontName = lf.lfFaceName;

    ZeroMemory(&WindowPlacement, sizeof(WindowPlacement));
}

void RegistrySettings::Load()
{
    LoadPresets();

    CRegKey view;
    if (view.Open(HKEY_CURRENT_USER, _T("Software\\Microsoft\\Windows\\CurrentVersion\\Applets\\Paint\\View"), KEY_READ) == ERROR_SUCCESS)
    {
        ReadDWORD(view, _T("BMPHeight"),     BMPHeight,     TRUE);
        ReadDWORD(view, _T("BMPWidth"),      BMPWidth,      TRUE);
        ReadDWORD(view, _T("GridExtent"),    GridExtent,    FALSE);
        ReadDWORD(view, _T("NoStretching"),  NoStretching,  FALSE);
        ReadDWORD(view, _T("ShowThumbnail"), ShowThumbnail, FALSE);
        ReadDWORD(view, _T("SnapToGrid"),    SnapToGrid,    FALSE);
        ReadDWORD(view, _T("ThumbHeight"),   ThumbHeight,   TRUE);
        ReadDWORD(view, _T("ThumbWidth"),    ThumbWidth,    TRUE);
        ReadDWORD(view, _T("ThumbXPos"),     ThumbXPos,     TRUE);
        ReadDWORD(view, _T("ThumbYPos"),     ThumbYPos,     TRUE);
        ReadDWORD(view, _T("UnitSetting"),   UnitSetting,   FALSE);

        ULONG pnBytes = sizeof(WINDOWPLACEMENT);
        view.QueryBinaryValue(_T("WindowPlacement"), &WindowPlacement, &pnBytes);
    }

    CRegKey files;
    if (files.Open(HKEY_CURRENT_USER, _T("Software\\Microsoft\\Windows\\CurrentVersion\\Applets\\Paint\\Recent File List"), KEY_READ) == ERROR_SUCCESS)
    {
        ReadString(files, _T("File1"), strFile1);
        ReadString(files, _T("File2"), strFile2);
        ReadString(files, _T("File3"), strFile3);
        ReadString(files, _T("File4"), strFile4);
    }

    CRegKey text;
    if (text.Open(HKEY_CURRENT_USER, _T("Software\\Microsoft\\Windows\\CurrentVersion\\Applets\\Paint\\Text"), KEY_READ) == ERROR_SUCCESS)
    {
        ReadDWORD(text, _T("Bold"),         Bold,           FALSE);
        ReadDWORD(text, _T("Italic"),       Italic,         FALSE);
        ReadDWORD(text, _T("Underline"),    Underline,      FALSE);
        ReadDWORD(text, _T("CharSet"),      CharSet,        FALSE);
        ReadDWORD(text, _T("PointSize"),    PointSize,      FALSE);
        ReadDWORD(text, _T("PositionX"),    FontsPositionX, FALSE);
        ReadDWORD(text, _T("PositionY"),    FontsPositionY, FALSE);
        ReadDWORD(text, _T("ShowTextTool"), ShowTextTool,   FALSE);
        ReadString(text, _T("TypeFaceName"), strFontName, strFontName);
    }

    // Fix the bitmap size if too large
    if (BMPWidth > 5000)
        BMPWidth = (GetSystemMetrics(SM_CXSCREEN) * 6) / 10;
    if (BMPHeight > 5000)
        BMPHeight = (GetSystemMetrics(SM_CYSCREEN) * 6) / 10;
}

void RegistrySettings::Store()
{
    CRegKey view;
    if (view.Create(HKEY_CURRENT_USER,
                     _T("Software\\Microsoft\\Windows\\CurrentVersion\\Applets\\Paint\\View")) == ERROR_SUCCESS)
    {
        view.SetDWORDValue(_T("BMPHeight"),     BMPHeight);
        view.SetDWORDValue(_T("BMPWidth"),      BMPWidth);
        view.SetDWORDValue(_T("GridExtent"),    GridExtent);
        view.SetDWORDValue(_T("NoStretching"),  NoStretching);
        view.SetDWORDValue(_T("ShowThumbnail"), ShowThumbnail);
        view.SetDWORDValue(_T("SnapToGrid"),    SnapToGrid);
        view.SetDWORDValue(_T("ThumbHeight"),   ThumbHeight);
        view.SetDWORDValue(_T("ThumbWidth"),    ThumbWidth);
        view.SetDWORDValue(_T("ThumbXPos"),     ThumbXPos);
        view.SetDWORDValue(_T("ThumbYPos"),     ThumbYPos);
        view.SetDWORDValue(_T("UnitSetting"),   UnitSetting);

        view.SetBinaryValue(_T("WindowPlacement"), &WindowPlacement, sizeof(WINDOWPLACEMENT));
    }

    CRegKey files;
    if (files.Create(HKEY_CURRENT_USER, _T("Software\\Microsoft\\Windows\\CurrentVersion\\Applets\\Paint\\Recent File List")) == ERROR_SUCCESS)
    {
        if (!strFile1.IsEmpty())
            files.SetStringValue(_T("File1"), strFile1);
        if (!strFile2.IsEmpty())
            files.SetStringValue(_T("File2"), strFile2);
        if (!strFile3.IsEmpty())
            files.SetStringValue(_T("File3"), strFile3);
        if (!strFile4.IsEmpty())
            files.SetStringValue(_T("File4"), strFile4);
    }

    CRegKey text;
    if (text.Create(HKEY_CURRENT_USER, _T("Software\\Microsoft\\Windows\\CurrentVersion\\Applets\\Paint\\Text")) == ERROR_SUCCESS)
    {
        text.SetDWORDValue(_T("Bold"),          Bold);
        text.SetDWORDValue(_T("Italic"),        Italic);
        text.SetDWORDValue(_T("Underline"),     Underline);
        text.SetDWORDValue(_T("CharSet"),       CharSet);
        text.SetDWORDValue(_T("PointSize"),     PointSize);
        text.SetDWORDValue(_T("PositionX"),     FontsPositionX);
        text.SetDWORDValue(_T("PositionY"),     FontsPositionY);
        text.SetDWORDValue(_T("ShowTextTool"),  ShowTextTool);
        text.SetStringValue(_T("TypeFaceName"), strFontName);
    }
}

void RegistrySettings::SetMostRecentFile(LPCTSTR szPathName)
{
    if (szPathName && szPathName[0])
        SHAddToRecentDocs(SHARD_PATHW, szPathName);

    if (strFile1 == szPathName)
    {
        // do nothing
    }
    else if (strFile2 == szPathName)
    {
        CString strTemp = strFile2;
        strFile2 = strFile1;
        strFile1 = strTemp;
    }
    else if (strFile3 == szPathName)
    {
        CString strTemp = strFile3;
        strFile3 = strFile2;
        strFile2 = strFile1;
        strFile1 = strTemp;
    }
    else if (strFile4 == szPathName)
    {
        CString strTemp = strFile4;
        strFile4 = strFile3;
        strFile3 = strFile2;
        strFile2 = strFile1;
        strFile1 = strTemp;
    }
    else
    {
        strFile4 = strFile3;
        strFile3 = strFile2;
        strFile2 = strFile1;
        strFile1 = szPathName;
    }
}
