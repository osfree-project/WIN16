#include <win16.h>

#define TD_SIGN 0x4454   /* "TD" = Task Database */
#define OFS_TD_SIGN 0xFA /* location of "TD" signature in Task DB */

//extern DWORD FAR PASCAL GetSelectorLimit(WORD w);

BOOL WINAPI IsTask(HTASK w)
{
  WORD far * lpwMaybeTask;

  if (!w)
    return FALSE;

  if (GetSelectorLimit(w) < (OFS_TD_SIGN+2))
    return FALSE;

  lpwMaybeTask=(WORD far *) MAKELP(w, OFS_TD_SIGN);

  return (*lpwMaybeTask == TD_SIGN);

}
