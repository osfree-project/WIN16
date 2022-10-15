#include <win16.h>

/***********************************************************************
 *           GetWindowsDirectory   (KERNEL.134)
 */
UINT WINAPI GetWindowsDirectory( LPSTR path, UINT count )
{
    lstrcpyn(path, "C:\\WINDOWS", count);
    return lstrlen(path);
}


/***********************************************************************
 *           GetSystemDirectory   (KERNEL.135)
 */
UINT WINAPI GetSystemDirectory( LPSTR path, UINT count )
{
    static const char system16[] = "\\SYSTEM";
    char windir[255]; //MAX_PATH
    UINT len;

    len = GetWindowsDirectory(windir, sizeof(windir) - sizeof(system16) + 1) + sizeof(system16);
    if (count >= len)
    {
        lstrcpy(path, windir);
        lstrcat(path, system16);
        len--;  /* space for the terminating zero is not included on success */
    }
    return len;
}
