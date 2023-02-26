#include <string.h>

#include <windows.h>

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

//    TRACE("inst=%04x id=%04x buff=%p len=%d\n", instance, resource_id, buffer, buflen);

    hrsrc = FindResource( instance, MAKEINTRESOURCE((resource_id>>4)+1), (LPSTR)RT_STRING );
    if (!hrsrc) return 0;
    hmem = LoadResource( instance, hrsrc );
    if (!hmem) return 0;

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
//        TRACE( "%s loaded\n", debugstr_a(buffer));
    }
    FreeResource( hmem );
    return ret;
}
