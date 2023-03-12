#include <windows.h>

DWORD WINAPI GetSystemMSecCount(void);

DWORD WINAPI GetCurrentTime()
{
    return GetSystemMSecCount();
}

DWORD WINAPI GetTickCount()
{
    return GetSystemMSecCount();
}
