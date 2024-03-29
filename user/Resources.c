#include <user.h>

/**********************************************************************
 *     LoadString   (USER.176)
 */
int WINAPI LoadString( HINSTANCE instance, UINT resource_id, LPSTR buffer, int buflen )
{
    HGLOBAL hmem;
    HRSRC hrsrc;
    unsigned char far *p;
    int string_num;
    int ret;

	FUNCTION_START
	TRACE("inst=%04x id=%04x buff=%p len=%d", instance, resource_id, buffer, buflen);

	hrsrc = FindResource( instance, MAKEINTRESOURCE((resource_id>>4)+1), RT_STRING );
	if (!hrsrc) 
	{
		TRACE("Resource not found");
		FUNCTION_END
		return 0;
	}

	hmem = LoadResource( instance, hrsrc );
	if (!hmem) 
	{
		TRACE("Error loading resource");
		FUNCTION_END
		return 0;
	}

	p = LockResource(hmem);
	string_num = resource_id & 0x000f;
	while (string_num--) p += *p + 1;

	if (buffer == NULL) ret = *p;
	else
	{
		ret = min(buflen - 1, *p);
		if (ret > 0)
		{
			_fmemcpy(buffer, p + 1, ret);
			buffer[ret] = '\0';
		}
		else if (buflen > 1)
		{
			buffer[0] = '\0';
			ret = 0;
		}
        TRACE( "%s loaded\n", buffer);
	}
	FreeResource( hmem );

	FUNCTION_END
	return ret;
}

/**********************************************************************
 *              LoadAccelerators  (USER.177)
 */
HACCEL	WINAPI
LoadAccelerators(HINSTANCE hInstance, LPCSTR lpTableName)
{
	HANDLE hResInfo;
	HACCEL rc;

	FUNCTION_START
	TRACE("LoadAccelerators(HINSTANCE=%x,LPCSTR=%x)",
		hInstance,lpTableName);

	hResInfo = FindResource(hInstance, lpTableName, RT_ACCELERATOR);

	if(hResInfo == 0) {
    		TRACE("LoadAccelerators: returns HACCEL 0");
		return 0;
	}

	rc =  LoadResource(hInstance,hResInfo);
    	TRACE("LoadAccelerators: returns HACCEL %x",rc);
	return rc;
}

/***********************************************************************
 *		LoadImage (USER.389)
 */
HANDLE WINAPI LoadImage(HINSTANCE hinst, LPCSTR name, UINT type, int cx, int cy, UINT flags)
{
    HGLOBAL handle;
    HRSRC hRsrc, hGroupRsrc;
    DWORD size;
/*
    if (!hinst || (flags & LR_LOADFROMFILE))
    {
        if (type == IMAGE_BITMAP)
            return HBITMAP_16( LoadImageA( 0, name, type, cx, cy, flags ));
        else
            return get_icon_16( LoadImageA( 0, name, type, cx, cy, flags ));
    }
*/
#if 0
    hinst = GetExePtr( hinst );

    if (flags & LR_DEFAULTSIZE)
    {
        if (type == IMAGE_ICON)
        {
            if (!cx) cx = GetSystemMetrics(SM_CXICON);
            if (!cy) cy = GetSystemMetrics(SM_CYICON);
        }
        else if (type == IMAGE_CURSOR)
        {
            if (!cx) cx = GetSystemMetrics(SM_CXCURSOR);
            if (!cy) cy = GetSystemMetrics(SM_CYCURSOR);
        }
    }

    switch (type)
    {
#if 0
// Huh... Save and load bitmap is not good idea... Better rewrite to create from memory
    case IMAGE_BITMAP:
    {
        HBITMAP ret = 0;
        char *ptr;
        static const char prefixW[] = {'b','m','p',0};
        BITMAPFILEHEADER header;
        char path[MAX_PATH], filename[MAX_PATH];
        HANDLE file;

        filename[0] = 0;
        if (!(hRsrc = FindResource( hinst, name, (LPCSTR)RT_BITMAP ))) return 0;
        if (!(handle = LoadResource( hinst, hRsrc ))) return 0;
        if (!(ptr = LockResource( handle ))) goto done;
        size = SizeofResource( hinst, hRsrc );

        header.bfType = 0x4d42; /* 'BM' */
        header.bfReserved1 = 0;
        header.bfReserved2 = 0;
        header.bfSize = sizeof(header) + size;
        header.bfOffBits = 0;  /* not used by the 32-bit loading code */

        if (!GetTempPath( MAX_PATH, path )) goto done;
        if (!GetTempFileName( path, prefixW, 0, filename )) goto done;

        file = CreateFile( filename, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, 0 );
        if (file != INVALID_HANDLE_VALUE)
        {
            DWORD written;
            BOOL ok;
            ok = WriteFile( file, &header, sizeof(header), &written, NULL ) && (written == sizeof(header));
            if (ok) ok = WriteFile( file, ptr, size, &written, NULL ) && (written == size);
            CloseHandle( file );
            if (ok) ret = LoadImage( 0, filename, IMAGE_BITMAP, cx, cy, flags | LR_LOADFROMFILE );
        }
    done:
        if (filename[0]) DeleteFile( filename );
        FreeResource( handle );
        return ret;
    }
#endif
    case IMAGE_ICON:
    case IMAGE_CURSOR:
    {
        HICON hIcon = 0;
        BYTE *dir, *bits;
        INT id = 0;

        if (!(hRsrc = FindResource( hinst, name,
                                      (LPCSTR)(type == IMAGE_ICON ? RT_GROUP_ICON : RT_GROUP_CURSOR ))))
            return 0;
        hGroupRsrc = hRsrc;

        if (!(handle = LoadResource( hinst, hRsrc ))) return 0;
        if ((dir = LockResource( handle ))) id = LookupIconIdFromDirectory( dir, type == IMAGE_ICON );
        FreeResource( handle );
        if (!id) return 0;

        if (!(hRsrc = FindResource( hinst, MAKEINTRESOURCEA(id),
                                      (LPCSTR)(type == IMAGE_ICON ? RT_ICON : RT_CURSOR) ))) return 0;

        if ((flags & LR_SHARED) && (hIcon = find_shared_icon( hinst, hRsrc ) ) != 0) return hIcon;

        if (!(handle = LoadResource( hinst, hRsrc ))) return 0;
        bits = LockResource( handle );
        size = SizeofResource( hinst, hRsrc );
/* @todo Here we need to parse resource and create icon
        hIcon = CreateIconFromResourceEx( bits, size, type == IMAGE_ICON, 0x00030000, cx, cy, flags );
*/
        FreeResource( handle );

/* @todo port shared icons code from wine */
//        if (hIcon && (flags & LR_SHARED)) add_shared_icon( hinst, hRsrc, hGroupRsrc, hIcon );
        return hIcon;
    }
    default:
        return 0;
    }
#endif
  return 0;
}

/**********************************************************************
 *		LoadBitmap (USER.175)
 */
HBITMAP WINAPI LoadBitmap(HINSTANCE hInstance, LPCSTR name)
{
    return 0;//return LoadImage( hInstance, name, IMAGE_BITMAP, 0, 0, 0 );
}

