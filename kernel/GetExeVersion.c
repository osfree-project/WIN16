#include <win16.h>

/***********************************************************************
 *           GetExeVersion   (KERNEL.105)
 */
WORD WINAPI GetExeVersion(void)
{
    return *((WORD far *)MAKELP(GetCurrentTask(), 0x1a));
}
