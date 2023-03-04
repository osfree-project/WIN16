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

#define GET_BYTE(ptr)  (*(const BYTE *)(ptr))
#define GET_WORD(ptr)  (*(const WORD *)(ptr))

static LPCSTR parse_menu_resource( LPCSTR res, HMENU hMenu, BOOL oldFormat )
{
    WORD flags, id = 0;
    LPCSTR str;
    BOOL end_flag;

    do
    {
        /* Windows 3.00 and later use a WORD for the flags, whereas 1.x and 2.x use a BYTE. */
        if (oldFormat)
        {
            flags = GET_BYTE(res);
            res += sizeof(BYTE);
        }
        else
        {
            flags = GET_WORD(res);
            res += sizeof(WORD);
        }

        end_flag = flags & MF_END;
        /* Remove MF_END because it has the same value as MF_HILITE */
        flags &= ~MF_END;
        if (!(flags & MF_POPUP))
        {
            id = GET_WORD(res);
            res += sizeof(WORD);
        }
        str = res;
        res += lstrlen(str) + 1;
        if (flags & MF_POPUP)
        {
            HMENU hSubMenu = CreatePopupMenu();
            if (!hSubMenu) return NULL;
            if (!(res = parse_menu_resource( res, hSubMenu, oldFormat ))) return NULL;
            AppendMenu( hMenu, flags, hSubMenu, str );
        }
        else  /* Not a popup */
        {
            AppendMenu( hMenu, flags, id, *str ? str : NULL );
        }
    } while (!end_flag);
    return res;
}

WORD WINAPI GetExeVersion();

/**********************************************************************
 *	    LoadMenuIndirect    (USER.220)
 */
HMENU WINAPI LoadMenuIndirect( VOID const far * template )
{
    BOOL oldFormat;
    HMENU hMenu;
    WORD version, offset;
    LPCSTR p = template;

//    TRACE("(%p)\n", template );

    /* Windows 1.x and 2.x menus have a slightly different menu format from 3.x menus */
    oldFormat = (GetExeVersion() < 0x0300);

    /* Windows 3.00 and later menu items are preceded by a MENUITEMTEMPLATEHEADER structure */
    if (!oldFormat)
    {
        version = GET_WORD(p);
        p += sizeof(WORD);
        if (version)
        {
//            WARN("version must be 0 for Win16 >= 3.00 applications\n" );
            return 0;
        }
        offset = GET_WORD(p);
        p += sizeof(WORD) + offset;
    }

    if (!(hMenu = CreateMenu())) return 0;
    if (!parse_menu_resource( p, hMenu, oldFormat ))
    {
        DestroyMenu( hMenu );
        return 0;
    }
    return hMenu;
}
