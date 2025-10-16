/*
 * File handling functions
 *
 * Copyright 1993 John Burton
 * Copyright 1996 Alexandre Julliard
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301, USA
 *
 */

#include <windows.h>

#include "win_private.h"


/***********************************************************************
 *           GetWindowsDirectory   (KERNEL.134)
 */
UINT WINAPI GetWindowsDirectory( LPSTR path, UINT count )
{
	FUNCTIONSTART;
    lstrcpyn(path, "C:\\WINDOWS", count);
	FUNCTIONEND;
    return lstrlen(path);
}


/***********************************************************************
 *           GetSystemDirectory   (KERNEL.135)
 */
UINT WINAPI GetSystemDirectory(LPSTR path, UINT count)
{
    static const char system16[] = "\\SYSTEM";
    char windir[255]; //MAX_PATH
    UINT len;

	FUNCTIONSTART;

    len = GetWindowsDirectory(windir, sizeof(windir) - sizeof(system16) + 1) + sizeof(system16);
    if (count >= len)
    {
        lstrcpy(path, windir);
        lstrcat(path, system16);
        len--;  /* space for the terminating zero is not included on success */
    }

	FUNCTIONEND;
    return len;
}


void WINAPI Dos3Call();

#define MOV_CX(x) __asm {mov cx, word ptr x}
#define MOV_BX(x) __asm {mov bx, word ptr x}
#define MOV_DX(x) __asm {mov dx, word ptr x}
#define MOV_DS(x) __asm {mov ds, word ptr x}
#define MOV_AL(x) __asm {mov al, byte ptr x}
#define MOV_AH_CONST(x) __asm {mov ah, x}

/* Safe AniToOem version */
void WINAPI SafeAnsiToOem(LPCSTR s, LPSTR d)
{
  //@todo check is keyboard driver present, if present, then use normal AnsiToOem, if not, just copy string.
  lstrcpy(d, s);
}


#pragma disable_message(107);
HFILE WINAPI _lopen(LPCSTR lpPathName, int iReadWrite)
{
	char Buf[128];
    __asm("push ds");
//	FUNCTIONSTART;
    SafeAnsiToOem(lpPathName, Buf);
    __asm {
		mov al, byte ptr iReadWrite
		mov dx, word ptr Buf
		mov ds, word ptr Buf+2
		mov ah, 3dh
        push cs                 /* make far call */
        call Dos3Call
		jnc lopenexit
		mov ax,-1
    lopenexit:
        pop ds
	}
    //	FUNCTIONEND;
}
#pragma enable_message(107);


#pragma disable_message(107);
HFILE WINAPI _lcreat(LPCSTR lpPathName, int a)
{
	char Buf[128];
	FUNCTIONSTART;
	SafeAnsiToOem(lpPathName, Buf);	
	__asm {
		push ds
		mov cx, a
		mov dx, word ptr Buf
		mov ds, word ptr Buf+2
		mov ax, 3ch
	}
	Dos3Call();
	__asm {
		pop ds
		jnc exit
		mov ax,-1
exit:
	}
	FUNCTIONEND;
}
#pragma enable_message(107);


#pragma disable_message(107);
HFILE WINAPI _lclose(HFILE h)
{
	FUNCTIONSTART;
	__asm {
		mov bx, h
		mov ah, 3eh
	}
	Dos3Call();
	__asm {
		mov ax,-1
		jc exit
		inc ax
exit:
	}
	FUNCTIONEND;
}
#pragma enable_message(107);


/***********************************************************************
 *           _lwrite   (KERNEL.86)
 */
#pragma disable_message(107);
UINT WINAPI _lwrite(HFILE hFile, const void __huge * buffer, UINT count )
{
	FUNCTIONSTART;
	__asm {
		push ds
		mov bx, hFile
		mov dx, word ptr buffer
		mov ds, word ptr buffer+2
		mov cx, count
		mov ah, 40h
	}
	Dos3Call();
	__asm {
		pop ds
		jnc exit
		mov ax,-1
exit:
	}
	FUNCTIONEND;
}
#pragma enable_message(107);


/***********************************************************************
 *           _llseek   (KERNEL.84)
 *
 * FIXME:
 *   Seeking before the start of the file should be allowed for _llseek,
 *   but cause subsequent I/O operations to fail (cf. interrupt list)
 *
 */
#pragma disable_message(107);
LONG WINAPI _llseek( HFILE hFile, LONG lOffset, int nOrigin )
{
	FUNCTIONSTART;
	__asm {
		mov bx, hFile
		mov dx, word ptr lOffset
		mov cs, word ptr lOffset
		mov ax, nOrigin
		mov ah, 42h
	}
	Dos3Call();
	__asm {
		jnc exit
		mov ax,-1
exit:
	}
	FUNCTIONEND;
}
#pragma enable_message(107);


/***********************************************************************
 *           _lread (KERNEL.82)
 */
#pragma disable_message(107);
UINT WINAPI _lread(HFILE hFile, void __huge * buffer, UINT count )
{
	FUNCTIONSTART;
	__asm {
		push ds
		mov bx, hFile
		mov dx, word ptr buffer
		mov ds, word ptr buffer+2
		mov cx, count
		mov ah, 3fh
	}
	Dos3Call();
	__asm {
		pop ds
		jnc exit
		mov ax,-1
exit:
	}
	FUNCTIONEND;
}
#pragma enable_message(107);

#if 0
/***********************************************************************
 *	GetTempPathW   (kernelbase.@)
 */
DWORD WINAPI DECLSPEC_HOTPATCH GetTempPathW( DWORD count, LPWSTR path )
{
    WCHAR tmp_path[MAX_PATH];
    UINT ret;

	FUNCTIONSTART;
	
    if (!(ret = GetEnvironmentVariableW( L"TMP", tmp_path, MAX_PATH )) &&
        !(ret = GetEnvironmentVariableW( L"TEMP", tmp_path, MAX_PATH )) &&
        !(ret = GetEnvironmentVariableW( L"USERPROFILE", tmp_path, MAX_PATH )) &&
        !(ret = GetWindowsDirectoryW( tmp_path, MAX_PATH )))
        return 0;

    if (ret > MAX_PATH)
    {
        SetLastError( ERROR_FILENAME_EXCED_RANGE );
        return 0;
    }
    ret = GetFullPathNameW( tmp_path, MAX_PATH, tmp_path, NULL );
    if (!ret) return 0;

    if (ret > MAX_PATH - 2)
    {
        SetLastError( ERROR_FILENAME_EXCED_RANGE );
        return 0;
    }
    if (tmp_path[ret-1] != '\\')
    {
        tmp_path[ret++] = '\\';
        tmp_path[ret]   = '\0';
    }

    ret++; /* add space for terminating 0 */
    if (count >= ret)
    {
        lstrcpynW( path, tmp_path, count );
        /* the remaining buffer must be zeroed up to 32766 bytes in XP or 32767
         * bytes after it, we will assume the > XP behavior for now */
        memset( path + ret, 0, (min(count, 32767) - ret) * sizeof(WCHAR) );
        ret--; /* return length without 0 */
    }
    else if (count)
    {
        /* the buffer must be cleared if contents will not fit */
        memset( path, 0, count * sizeof(WCHAR) );
    }

    FUNCTIONEND;
    return ret;
}

/***********************************************************************
 *           GetTempDrive   (KERNEL.92)
 * A closer look at krnl386.exe shows what the SDK doesn't mention:
 *
 * returns:
 *   AL: driveletter
 *   AH: ':'		- yes, some kernel code even does stosw with
 *                            the returned AX.
 *   DX: 1 for success
 */
UINT WINAPI GetTempDrive( BYTE ignored )
{
    WCHAR buffer[MAX_PATH];
    BYTE ret = 'C';

    if (GetTempPathW( MAX_PATH, buffer ))
    {
        ret = buffer[0];
        if (ret >= 'a' && ret <= 'z') ret += 'A' - 'a';
    }
    return MAKELONG( ret | (':' << 8), 1 );
}


/***********************************************************************
 *           GetTempFileName   (KERNEL.97)
 */
UINT16 WINAPI GetTempFileName16( BYTE drive, LPCSTR prefix, UINT16 unique,
                                 LPSTR buffer )
{
    char temppath[MAX_PATH];
    char *prefix16 = NULL;
    UINT16 ret;

    if (!(drive & ~TF_FORCEDRIVE)) /* drive 0 means current default drive */
    {
        GetCurrentDirectoryA(sizeof(temppath), temppath); 
        drive |= temppath[0];
    }

    if (drive & TF_FORCEDRIVE)
    {
        char    d[3];

        d[0] = drive & ~TF_FORCEDRIVE;
        d[1] = ':';
        d[2] = '\0';
        if (GetDriveTypeA(d) == DRIVE_NO_ROOT_DIR)
        {
            drive &= ~TF_FORCEDRIVE;
            WARN("invalid drive %d specified\n", drive );
        }
    }

    if (drive & TF_FORCEDRIVE)
        sprintf(temppath,"%c:", drive & ~TF_FORCEDRIVE );
    else
        GetTempPathA( MAX_PATH, temppath );

    if (prefix)
    {
        prefix16 = HeapAlloc(GetProcessHeap(), 0, strlen(prefix) + 2);
        *prefix16 = '~';
        strcpy(prefix16 + 1, prefix);
    }

    ret = GetTempFileNameA( temppath, prefix16, unique, buffer );

    HeapFree(GetProcessHeap(), 0, prefix16);
    return ret;
}

/***********************************************************************
 *	GetTempFileNameW   (kernelbase.@)
 */
UINT WINAPI DECLSPEC_HOTPATCH GetTempFileNameW( LPCWSTR path, LPCWSTR prefix, UINT unique, LPWSTR buffer )
{
    int i;
    LPWSTR p;
    DWORD attr;

    if (!path || !buffer)
    {
        SetLastError( ERROR_INVALID_PARAMETER );
        return 0;
    }

    /* ensure that the provided directory exists */
    attr = GetFileAttributesW( path );
    if (attr == INVALID_FILE_ATTRIBUTES || !(attr & FILE_ATTRIBUTE_DIRECTORY))
    {
        TRACE( "path not found %s\n", debugstr_w( path ));
        SetLastError( ERROR_DIRECTORY );
        return 0;
    }

    lstrcpyW( buffer, path );
    p = buffer + lstrlenW(buffer);

    /* add a \, if there isn't one  */
    if ((p == buffer) || (p[-1] != '\\')) *p++ = '\\';

    if (prefix) for (i = 3; (i > 0) && (*prefix); i--) *p++ = *prefix++;

    unique &= 0xffff;
    if (unique) swprintf( p, MAX_PATH - (p - buffer), L"%x.tmp", unique );
    else
    {
        /* get a "random" unique number and try to create the file */
        HANDLE handle;
        UINT num = NtGetTickCount() & 0xffff;
        static UINT last;

        /* avoid using the same name twice in a short interval */
        if (last - num < 10) num = last + 1;
        if (!num) num = 1;
        unique = num;
        do
        {
            swprintf( p, MAX_PATH - (p - buffer), L"%x.tmp", unique );
            handle = CreateFileW( buffer, GENERIC_WRITE, 0, NULL, CREATE_NEW, FILE_ATTRIBUTE_NORMAL, 0 );
            if (handle != INVALID_HANDLE_VALUE)
            {  /* We created it */
                CloseHandle( handle );
                last = unique;
                break;
            }
            if (GetLastError() != ERROR_FILE_EXISTS && GetLastError() != ERROR_SHARING_VIOLATION)
                break;  /* No need to go on */
            if (!(++unique & 0xffff)) unique = 1;
        } while (unique != num);
    }
    TRACE( "returning %s\n", debugstr_w( buffer ));
    return unique;
}

GetCurrentDirectoryA proc public uses ebx esi edi maxlen:dword,buffer:ptr byte

local   tmpbuf[MAXPATH]:byte

;*** the buffer may be very small. In this
;*** case, GetCurrentDirectory returns the requested size of the buffer

	mov ah,19h			   ;get drive
	int 21h
	mov dl,al
	inc dl
	add al,'A'
	lea edi,tmpbuf		   ;store it in tmpbuf
	mov ah,':'
	stosw
	mov al,'\'
	stosb

	mov esi,edi
	mov ax,7147h
	stc
	int 21h
	jnc success
	cmp ax,7100h
	jnz error
	mov ah,47h
	int 21h
	jc error
success:
	mov al,00
	mov ecx,-1
	repnz scasb
	mov eax,edi
	lea ecx,tmpbuf
	sub eax,ecx 		;required size of buffer
	cmp eax,maxlen
	ja exit				;buffer too small, return req. size
	mov ecx,eax
	lea esi,tmpbuf
	mov edi,buffer
	rep movsb
	@strace <"GetCurrentDirectoryA: [buffer]=", &buffer>
	dec eax 			;do not count last '00'
	jmp exit
error:
	movzx eax, al
	invoke SetLastError, eax
	xor eax,eax
exit:
	@strace <"GetCurrentDirectoryA(", maxlen, ", ", buffer, ")=", eax>
	ret
	align 4

GetCurrentDirectoryA endp

#endif
