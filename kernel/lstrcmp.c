
#include <win16.h>

int strcmp (const char far *s1, const char far *s2)
{
  while (*s1 || *s2)
    {
      if (*s1 < *s2)
        return -1;
      else if (*s1 > *s2)
        return 1;
      s1 ++;
      s2 ++;
    }

  return 0;
}

/***********************************************************************
 *		Reserved5 (KERNEL.87)
 */
int WINAPI lstrcmp( LPCSTR str1, LPCSTR str2 )
{
    int ret = strcmp( str1, str2 );

    /* Looks too complicated, but in optimized strcpy we might get
     * a 32bit wide difference and would truncate it to 16 bit, so
     * erroneously returning equality. */
    if (ret < 0) return -1;
    if (ret > 0) return  1;
    return 0;
}
