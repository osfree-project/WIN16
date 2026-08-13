
#include <windows.h>


/***************************************************************************
 *          IsRomModule    (KERNEL.323)
 */
BOOL WINAPI IsRomModule( HMODULE unused )
{
    return FALSE;
}

/***************************************************************************
 *          IsRomFile    (KERNEL.326)
 */
BOOL WINAPI IsRomFile( HFILE unused )
{
    return FALSE;
}
