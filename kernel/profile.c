/*
 * INI file handling functions
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

//#include <i86.h>
#include <windows.h>

#include <win_private.h>

#define CR 13
#define LF 10
#define BUFFER_SIZE 0xFFF0

static int find_char_forward(LPCSTR str, int count, char ch) {
    int i;
    for (i = 0; i < count; i++) {
        if (str[i] == ch) return i;
    }
    return -1;
}

static void get_all_keys(char FAR** src, char FAR* dest, int* count, int* buf_remaining) {
    char c;
    while (*buf_remaining > 0 && *count > 0) {
        c = *(*src)++;
        (*count)--;
        if (c == '=') {
            *dest = '\0';
            while (*count > 0) {
                c = *(*src)++;
                (*count)--;
                if (c == LF) {
                    c = **src;
                    if (c == ';' || c == ' ' || c == '[') continue;
                    break;
                }
            }
            return;
        }
        *dest++ = c;
        (*buf_remaining)--;
    }
    *dest = '\0';
}

static int search_section(char FAR ** buffer, int* count, LPCSTR section) {
    char FAR* p = *buffer;
    int len = lstrlen(section);
    
    while (*count > 0) {
        /* Find newline */
        while (*count > 0 && *p != LF) {
            p++;
            (*count)--;
        }
        if (*count == 0) break;
        
        p++; /* Skip LF */
        (*count)--;
        
        /* Check for section start */
        if (*count > 0 && *p == '[') {
            char FAR* end = p + 1;
            int sec_len = 0;
            int remaining = *count;
            
            /* Find closing bracket */
            while (remaining > 0 && *end != ']' && *end != LF && sec_len < len + 10) {
                end++;
                sec_len++;
                remaining--;
            }
            
            if (remaining > 0 && *end == ']' && 
                strnicmp(p + 1, section, len) == 0) {
                *buffer = end + 1;
                *count = remaining - 1;
                return 1;
            }
        }
    }
    return 0;
}

static int search_entry(char FAR** buffer, int* count, LPCSTR entry) {
    char FAR* p = *buffer;
    int len = lstrlen(entry);
    
    while (*count > 0) {
        /* Find newline */
        while (*count > 0 && *p != LF) {
            p++;
            (*count)--;
        }
        if (*count == 0) break;
        
        p++; /* Skip LF */
        (*count)--;
        
        /* Check for new section */
        if (*count > 0 && *p == '[') return 0;
        
        /* Check for entry */
        if (*count >= len + 1) {
            char FAR* eq = p + len;
            if (strnicmp(p, entry, len) == 0 && *eq == '=') {
                *buffer = eq + 1;
                *count -= (len + 1);
                return 1;
            }
        }
    }
    return 0;
}

/* Main DLL functions */
int FAR PASCAL GetPrivateProfileString(LPCSTR lpszSection, LPCSTR lpszEntry,
                                       LPCSTR lpszDefault, LPSTR retBuffer,
                                       int bufSize, LPCSTR lpszFilename)
{
    HFILE hFile;
    int rc = 0;
    HANDLE hBuffer;
    char FAR* buffer;
    UINT file_size;
    int count;
    char FAR* ptr;
    
    /* Validate parameters */
    if (!retBuffer || bufSize == 0) return 0;
    retBuffer[0] = '\0';
    printf("%d section=%S entry=%S default=%S filename=%S\n\r", sizeof(LPCSTR),lpszSection, lpszEntry, lpszDefault, lpszFilename);

    /* Allocate buffer using GlobalAlloc */
    hBuffer = GlobalAlloc(GMEM_MOVEABLE | GMEM_ZEROINIT, BUFFER_SIZE);
    if (!hBuffer) {
        if (lpszDefault) {
            lstrcpyn(retBuffer, lpszDefault, bufSize);
            return lstrlen(retBuffer);
        }
        return 0;
    }
    
    buffer = GlobalLock(hBuffer);
    if (!buffer) {
        GlobalFree(hBuffer);
        if (lpszDefault) {
            lstrcpyn(retBuffer, lpszDefault, bufSize);
            return lstrlen(retBuffer);
        }
        return 0;
    }

    /* Open file with _lopen */
    hFile = _lopen(lpszFilename, OF_READ);
    if (hFile == HFILE_ERROR) {
        GlobalUnlock(hBuffer);
        GlobalFree(hBuffer);
        if (lpszDefault) {
            lstrcpyn(retBuffer, lpszDefault, bufSize);
            return lstrlen(retBuffer);
        }
        return 0;
    }

    /* Read file */
    file_size = _lread(hFile, buffer, BUFFER_SIZE - 1);
    _lclose(hFile);
    
    if (file_size == HFILE_ERROR || file_size == 0) {
        GlobalUnlock(hBuffer);
        GlobalFree(hBuffer);
        if (lpszDefault) {
            lstrcpyn(retBuffer, lpszDefault, bufSize);
            return lstrlen(retBuffer);
        }
        return 0;
    }
    
    /* Null-terminate the buffer */
    buffer[file_size] = '\0';
    count = file_size;
    ptr = buffer;
    
    /* Search for section */
    if (!search_section(&ptr, &count, lpszSection)) {
        GlobalUnlock(hBuffer);
        GlobalFree(hBuffer);
        if (lpszDefault) {
            lstrcpyn(retBuffer, lpszDefault, bufSize);
            return lstrlen(retBuffer);
        }
        return 0;
    }
    
    /* Special case: get all keys in section */
    if (lpszEntry == NULL || lpszEntry[0] == '\0') {
        char FAR * line_start = ptr;
        int found_any = 0;
        
        while (count > 0 && *ptr != '[') {
            if (*ptr == '=') {
                /* Found a key-value pair */
                char FAR* dest = retBuffer + rc;
                int copied = 0;
                int space_remaining = bufSize - rc - 1; /* -1 for null terminator */
                
                if (found_any && space_remaining > 0) {
                    *dest++ = '\0'; /* Separate keys with null character */
                    rc++;
                    space_remaining--;
                }
                
                while (line_start < ptr && copied < space_remaining) {
                    *dest++ = *line_start++;
                    copied++;
                }
                *dest = '\0';
                rc += copied;
                found_any = 1;
            }
            
            if (*ptr == LF) {
                line_start = ptr + 1;
            }
            ptr++;
            count--;
        }
        
        /* Ensure double null termination for multiple keys */
        if (rc < bufSize) {
            retBuffer[rc] = '\0';
        }
    }
    /* Search for specific entry */
    else {
        int rem;
        char FAR* p;

        if (!search_entry(&ptr, &count, lpszEntry)) {
            GlobalUnlock(hBuffer);
            GlobalFree(hBuffer);
            if (lpszDefault) {
                lstrcpyn(retBuffer, lpszDefault, bufSize);
                return lstrlen(retBuffer);
            }
            return 0;
        }
        
        /* Copy value */
        rem = bufSize;
        get_all_keys(&ptr, retBuffer, &count, &rem);
        
        /* Calculate length */
        p = retBuffer;
        while (*p && rc < bufSize) {
            rc++;
            p++;
        }
    }
    
    GlobalUnlock(hBuffer);
    GlobalFree(hBuffer);
    return rc;
}

int FAR PASCAL WritePrivateProfileString(LPCSTR lpszSection, LPCSTR lpszEntry,
                                         LPCSTR lpszString, LPCSTR lpszFilename)
{
    HFILE hFile;
    HANDLE hBuffer;
    char FAR* buffer;
    UINT file_size;
    UINT rc = 0;
    
    /* Validate parameters */
    if (!lpszSection || !lpszFilename) return 0;
    
    /* Allocate buffer */
    hBuffer = GlobalAlloc(GMEM_MOVEABLE | GMEM_ZEROINIT, BUFFER_SIZE);
    if (!hBuffer) return 0;
    
    buffer = GlobalLock(hBuffer);
    if (!buffer) {
        GlobalFree(hBuffer);
        return 0;
    }
    
    /* Try to open existing file */
    hFile = _lopen(lpszFilename, OF_READ);
    if (hFile != HFILE_ERROR) {
        file_size = _lread(hFile, buffer, BUFFER_SIZE - 1);
        _lclose(hFile);
        if (file_size != HFILE_ERROR) {
            buffer[file_size] = '\0';
        } else {
            file_size = 0;
            buffer[0] = '\0';
        }
    } else {
        /* File doesn't exist - start with empty buffer */
        file_size = 0;
        buffer[0] = '\0';
    }
    
    /* Simple implementation: always append new section/entry */
    /* Full INI file parsing and modification is complex */
    if (file_size > 0 && buffer[file_size - 1] != LF && buffer[file_size - 1] != CR) {
        /* Add newline if missing */
        if (file_size < BUFFER_SIZE - 2) {
            buffer[file_size++] = CR;
            buffer[file_size++] = LF;
        }
    }
    
    /* Write section header */
    if (file_size < BUFFER_SIZE - 3) {
        buffer[file_size++] = '[';
    }
    
    /* Copy section name */
    if (lpszSection) {
        int i = 0;
        while (lpszSection[i] && file_size < BUFFER_SIZE - 3) {
            buffer[file_size++] = lpszSection[i++];
        }
    }
    
    if (file_size < BUFFER_SIZE - 3) {
        buffer[file_size++] = ']';
        buffer[file_size++] = CR;
        buffer[file_size++] = LF;
    }
    
    /* Write entry if provided */
    if (lpszEntry && lpszString) {
        /* Copy entry name */
        int i = 0;
        while (lpszEntry[i] && file_size < BUFFER_SIZE - 3) {
            buffer[file_size++] = lpszEntry[i++];
        }
        
        if (file_size < BUFFER_SIZE - 3) {
            buffer[file_size++] = '=';
        }
        
        /* Copy value */
        i = 0;
        while (lpszString[i] && file_size < BUFFER_SIZE - 3) {
            buffer[file_size++] = lpszString[i++];
        }
        
        if (file_size < BUFFER_SIZE - 3) {
            buffer[file_size++] = CR;
            buffer[file_size++] = LF;
        }
    }
    
    /* Ensure null termination */
    if (file_size < BUFFER_SIZE) {
        buffer[file_size] = '\0';
    } else {
        buffer[BUFFER_SIZE - 1] = '\0';
        file_size = BUFFER_SIZE - 1;
    }
    
    /* Create or overwrite file */
    hFile = _lcreat(lpszFilename, 0);
    if (hFile != HFILE_ERROR) {
        /* Write buffer to file */
        UINT written = _lwrite(hFile, buffer, file_size);
        _lclose(hFile);
        if (written == file_size) {
            rc = 1; /* Success */
        }
    }
    
    GlobalUnlock(hBuffer);
    GlobalFree(hBuffer);
    return rc;
}

/***********************************************************************
 *           GetProfileInt   (KERNEL.57)
 */
UINT WINAPI GetProfileInt( LPCSTR section, LPCSTR entry, int def_val )
{
    UINT res;
    FUNCTIONSTART;

    res=GetPrivateProfileInt( section, entry, def_val, "win.ini" );

    FUNCTIONEND;
    return res;
}

/***********************************************************************
 *           GetPrivateProfileInt   (KERNEL.127)
 */
UINT WINAPI GetPrivateProfileInt( LPCSTR section, LPCSTR entry,
                                      int def_val, LPCSTR filename )
{
    char buffer[30];

    FUNCTIONSTART;

    /* we used to have some elaborate return value limitation (<= -32768 etc.)
     * here, but Win98SE doesn't care about this at all, so I deleted it.
     * AFAIR versions prior to Win9x had these limits, though. */


    if (GetPrivateProfileString( section, entry, "", buffer, sizeof( buffer ), filename ) == 0)
    {
        FUNCTIONEND;
        return def_val;
    }

    /* FIXME: if entry can be found but it's empty, then Win16 is
     * supposed to return 0 instead of def_val ! Difficult/problematic
     * to implement (every other failure also returns zero buffer),
     * thus wait until testing framework avail for making sure nothing
     * else gets broken that way. */
    if (!buffer[0]) 
    {
        FUNCTIONEND;
        return (UINT)def_val;
    }

    FUNCTIONEND;
    return atoi(buffer);
}

/***********************************************************************
 *           GetProfileString   (KERNEL.58)
 */
int WINAPI GetProfileString( LPCSTR section, LPCSTR entry, LPCSTR def_val,
                                 LPSTR buffer, int len )
{
    int res;

    FUNCTIONSTART;

    res=GetPrivateProfileString( section, entry, def_val,
                                      buffer, len, "win.ini");

    FUNCTIONEND;
    return res;
}


/***********************************************************************
 *           WriteProfileString   (KERNEL.59)
 */
BOOL WINAPI WriteProfileString( LPCSTR section, LPCSTR entry,
                                    LPCSTR string )
{
    BOOL res;
    FUNCTIONSTART;

    res=WritePrivateProfileString( section, entry, string, "win.ini" );

    FUNCTIONEND;
    return res;
}
