
#include <win16.h>

LPSTR WINAPI lstrcpyn( LPSTR dst, LPCSTR src, int n )
{
    /* Note: this function differs from the UNIX strncpy, it _always_ writes
     * a terminating \0.
     *
     * Note: n is an INT but Windows treats it as unsigned, and will happily
     * copy a gazillion chars if n is negative.
     */
//    __TRY
    {
        LPSTR d = dst;
        LPCSTR s = src;
        UINT count = n;

        while ((count > 1) && *s)
        {
            count--;
            *d++ = *s++;
        }
        if (count) *d = 0;
    }
//    __EXCEPT_PAGE_FAULT
//    {
//        SetLastError( ERROR_INVALID_PARAMETER );
//        return 0;
//    }
//    __ENDTRY
    return dst;
}
