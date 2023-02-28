#include <ctype.h>

#include <windows.h>

int latoi(char far *h)
{
  char far *s = h;
  int  i = 0;
  int  j, k, l;
  char c;
  int  base;

  if (s[0] == '0' && s[1] == 'x') {
    base = 16;
    s += 2; // Delete "0x"
  } else {
    base = 10;
  }

  l = lstrlen(s) - 1;

  while (*s) {
    c = tolower(*s);

    if ('a' <= c && c <= 'f') {
      if (base == 16) {
        c = c - 'a' + 10;
      } else {
        return 0;
      }
    } else if ('0' <= c && c <= '9') {
      c -= '0';
    } else {
      return 0;
    }

    for (j = 0, k = c; j < l; j++)
      k *= base;

    i += k;
    s++;
    l--;
  }

  return i;
}

HMODULE WINAPI GetExePtr( HANDLE handle );

/**********************************************************************
 *	    LoadMenu    (USER.150)
 */
HMENU WINAPI LoadMenu( HINSTANCE instance, LPCSTR name )
{
    HRSRC hRsrc;
    HGLOBAL handle;
    HMENU hMenu;

    if (HIWORD(name) && name[0] == '#') name = (char far *)latoi( (char far *)name + 1 );
    if (!name) return 0;

    instance = GetExePtr( instance );
    if (!(hRsrc = FindResource( instance, name, (LPSTR)RT_MENU ))) return 0;
    if (!(handle = LoadResource( instance, hRsrc ))) return 0;
    hMenu = LoadMenuIndirect(LockResource(handle));
    FreeResource( handle );
    return hMenu;
}

/*******************************************************************
 *         ChangeMenu    (USER.153)
 */
BOOL WINAPI ChangeMenu( HMENU hMenu, UINT pos, const char far * data,
                            UINT id, UINT flags )
{
    if (flags & MF_APPEND) return AppendMenu( hMenu, flags & ~MF_APPEND, id, data );

    /* FIXME: Word passes the item id in 'pos' and 0 or 0xffff as id */
    /* for MF_DELETE. We should check the parameters for all others */
    /* MF_* actions also (anybody got a doc on ChangeMenu?). */

    if (flags & MF_DELETE) return DeleteMenu(hMenu, pos, flags & ~MF_DELETE);
    if (flags & MF_CHANGE) return ModifyMenu(hMenu, pos, flags & ~MF_CHANGE, id, data );
    if (flags & MF_REMOVE) return RemoveMenu(hMenu, flags & MF_BYPOSITION ? pos : id,
                                               flags & ~MF_REMOVE );
    /* Default: MF_INSERT */
    return InsertMenu( hMenu, pos, flags, id, data );
}