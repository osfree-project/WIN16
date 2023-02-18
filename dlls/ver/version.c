/*
 * Implementation of VER.DLL
 *
 * Copyright 1996, 1997 Marcus Meissner
 * Copyright 1997 David Cuthbert
 * Copyright 1999 Ulrich Weigand
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
 */

#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <dos.h>
#include <ctype.h>
#include <sys/types.h>

#define NONAMELESSUNION
#define NONAMELESSSTRUCT
#include <windows.h>
#include <lzexpand.h>
#include <ver.h>


#define GlobalPtrHandle(lp) \
  ((HGLOBAL)LOWORD(GlobalHandle(SELECTOROF(lp))))

#define     GlobalUnlockPtr(lp)      \
                GlobalUnlock(GlobalPtrHandle(lp))

#define GlobalFreePtr(lp) \
  (GlobalUnlockPtr(lp),(BOOL)GlobalFree(GlobalPtrHandle(lp)))

#define GlobalAllocPtr(flags, cb) \
  (GlobalLock(GlobalAlloc((flags), (cb))))

//WINE_DEFAULT_DEBUG_CHANNEL(ver);

#pragma pack(push,4)


/*
 * Resource directory stuff
 */
typedef struct _IMAGE_RESOURCE_DIRECTORY {
	DWORD	Characteristics;
	DWORD	TimeDateStamp;
	WORD	MajorVersion;
	WORD	MinorVersion;
	WORD	NumberOfNamedEntries;
	WORD	NumberOfIdEntries;
	/*  IMAGE_RESOURCE_DIRECTORY_ENTRY DirectoryEntries[]; */
} IMAGE_RESOURCE_DIRECTORY,*PIMAGE_RESOURCE_DIRECTORY;

#define	IMAGE_RESOURCE_NAME_IS_STRING		0x80000000
#define	IMAGE_RESOURCE_DATA_IS_DIRECTORY	0x80000000

typedef struct _IMAGE_RESOURCE_DIRECTORY_ENTRY {
	union {
		struct {
			unsigned long NameOffset:31;
			unsigned long NameIsString:1;
		} s;
		DWORD   Name;
		WORD    Id;
	} u;
	union {
		DWORD   OffsetToData;
		struct {
			unsigned long OffsetToDirectory:31;
			unsigned long DataIsDirectory:1;
		} s2;
	} u2;
} IMAGE_RESOURCE_DIRECTORY_ENTRY,*PIMAGE_RESOURCE_DIRECTORY_ENTRY;


/* Image resource directory string */
typedef struct _IMAGE_RESOURCE_DIRECTORY_STRING {
    WORD    Length;
    char    NameString[1];
} IMAGE_RESOURCE_DIRECTORY_STRING;
typedef IMAGE_RESOURCE_DIRECTORY_STRING *PIMAGE_RESOURCE_DIRECTORY_STRING;

#pragma pack(pop)

/* Image resource data entry */
typedef struct _IMAGE_RESOURCE_DATA_ENTRY {
    DWORD   OffsetToData;
    DWORD   Size;
    DWORD   CodePage;
    DWORD   Reserved;
} IMAGE_RESOURCE_DATA_ENTRY;
typedef IMAGE_RESOURCE_DATA_ENTRY   *PIMAGE_RESOURCE_DATA_ENTRY;

typedef struct _IMAGE_FILE_HEADER {
  WORD  Machine;
  WORD  NumberOfSections;
  DWORD TimeDateStamp;
  DWORD PointerToSymbolTable;
  DWORD NumberOfSymbols;
  WORD  SizeOfOptionalHeader;
  WORD  Characteristics;
} IMAGE_FILE_HEADER, *PIMAGE_FILE_HEADER;

/* Image data directory */
typedef struct _IMAGE_DATA_DIRECTORY {
    DWORD   VirtualAddress;
    DWORD   Size;
} IMAGE_DATA_DIRECTORY;
typedef IMAGE_DATA_DIRECTORY    *PIMAGE_DATA_DIRECTORY;

#define IMAGE_NUMBEROF_DIRECTORY_ENTRIES    16

/* Optional header for PE image files (32-bit version) */
typedef struct _IMAGE_OPTIONAL_HEADER {
    WORD                    Magic;
    BYTE                    MajorLinkerVersion;
    BYTE                    MinorLinkerVersion;
    DWORD                   SizeOfCode;
    DWORD                   SizeOfInitializedData;
    DWORD                   SizeOfUninitializedData;
    DWORD                   AddressOfEntryPoint;
    DWORD                   BaseOfCode;
    DWORD                   BaseOfData;
    DWORD                   ImageBase;
    DWORD                   SectionAlignment;
    DWORD                   FileAlignment;
    WORD                    MajorOperatingSystemVersion;
    WORD                    MinorOperatingSystemVersion;
    WORD                    MajorImageVersion;
    WORD                    MinorImageVersion;
    WORD                    MajorSubsystemVersion;
    WORD                    MinorSubsystemVersion;
    DWORD                   Win32VersionValue;
    DWORD                   SizeOfImage;
    DWORD                   SizeOfHeaders;
    DWORD                   CheckSum;
    WORD                    Subsystem;
    WORD                    DllCharacteristics;
    DWORD                   SizeOfStackReserve;
    DWORD                   SizeOfStackCommit;
    DWORD                   SizeOfHeapReserve;
    DWORD                   SizeOfHeapCommit;
    DWORD                   LoaderFlags;
    DWORD                   NumberOfRvaAndSizes;
    IMAGE_DATA_DIRECTORY    DataDirectory[IMAGE_NUMBEROF_DIRECTORY_ENTRIES];
} IMAGE_OPTIONAL_HEADER32;
typedef IMAGE_OPTIONAL_HEADER32 *PIMAGE_OPTIONAL_HEADER32;

#define IMAGE_DIRECTORY_ENTRY_RESOURCE          2

#define IMAGE_SIZEOF_SHORT_NAME 8

typedef struct _IMAGE_SECTION_HEADER {
  BYTE  Name[IMAGE_SIZEOF_SHORT_NAME];
  union {
    DWORD PhysicalAddress;
    DWORD VirtualSize;
  } Misc;
  DWORD VirtualAddress;
  DWORD SizeOfRawData;
  DWORD PointerToRawData;
  DWORD PointerToRelocations;
  DWORD PointerToLinenumbers;
  WORD  NumberOfRelocations;
  WORD  NumberOfLinenumbers;
  DWORD Characteristics;
} IMAGE_SECTION_HEADER, far *PIMAGE_SECTION_HEADER;

typedef struct _IMAGE_NT_HEADERS {
  DWORD Signature; /* "PE"\0\0 */	/* 0x00 */
  IMAGE_FILE_HEADER FileHeader;		/* 0x04 */
  IMAGE_OPTIONAL_HEADER32 OptionalHeader;	/* 0x18 */
} IMAGE_NT_HEADERS, *PIMAGE_NT_HEADERS;

#define RT_VERSION          MAKEINTRESOURCE( 16 )

#define INVALID_FILE_ATTRIBUTES     0xFFFF

/***********************************************************************
 * Version Info Structure
 */

typedef struct
{
    WORD  wLength;
    WORD  wValueLength;
    char  szKey[1];
#if 0   /* variable length structure */
    /* DWORD aligned */
    BYTE  Value[];
    /* DWORD aligned */
    VS_VERSION_INFO_STRUCT16 Children[];
#endif
} VS_VERSION_INFO_STRUCT16;

/**********************************************************************
 *  find_entry_by_id
 *
 * Find an entry by id in a resource directory
 * Copied from loader/pe_resource.c
 */
static const IMAGE_RESOURCE_DIRECTORY far *find_entry_by_id( const IMAGE_RESOURCE_DIRECTORY far *dir,
                                                         WORD id, const void far *root )
{
    const IMAGE_RESOURCE_DIRECTORY_ENTRY far *entry;
    int min, max, pos;

    entry = (const IMAGE_RESOURCE_DIRECTORY_ENTRY far *)(dir + 1);
    min = dir->NumberOfNamedEntries;
    max = min + dir->NumberOfIdEntries - 1;
    while (min <= max)
    {
        pos = (min + max) / 2;
        if (entry[pos].u.Id == id)
            return (const IMAGE_RESOURCE_DIRECTORY far *)((const char far *)root + entry[pos].u2.s2.OffsetToDirectory);
        if (entry[pos].u.Id > id) max = pos - 1;
        else min = pos + 1;
    }
    return NULL;
}


/**********************************************************************
 *  find_entry_default
 *
 * Find a default entry in a resource directory
 * Copied from loader/pe_resource.c
 */
static const IMAGE_RESOURCE_DIRECTORY far *find_entry_default( const IMAGE_RESOURCE_DIRECTORY far *dir,
                                                           const void far *root )
{
    const IMAGE_RESOURCE_DIRECTORY_ENTRY far *entry;

    entry = (const IMAGE_RESOURCE_DIRECTORY_ENTRY far *)(dir + 1);
    return (const IMAGE_RESOURCE_DIRECTORY far *)((const char far *)root + entry->u2.s2.OffsetToDirectory);
}


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

/**********************************************************************
 *  find_entry_by_name
 *
 * Find an entry by name in a resource directory
 * Copied from loader/pe_resource.c
 */
static const IMAGE_RESOURCE_DIRECTORY far *find_entry_by_name( const IMAGE_RESOURCE_DIRECTORY far *dir,
                                                           LPCSTR name, const void far *root )
{
    const IMAGE_RESOURCE_DIRECTORY far *ret = NULL;
    DWORD namelen=lstrlen(name);

    if (!HIWORD(name)) return find_entry_by_id( dir, LOWORD(name), root );
    if (name[0] == '#')
    {
        return find_entry_by_id( dir, latoi((LPSTR)name+1), root );
    }

//    if ((nameW = HeapAlloc( GetProcessHeap(), 0, namelen * sizeof(WCHAR) )))
    {
        const IMAGE_RESOURCE_DIRECTORY_ENTRY *entry;
        const IMAGE_RESOURCE_DIRECTORY_STRING *str;
        int min, max, res, pos;

//        MultiByteToWideChar( CP_ACP, 0, name, -1, nameW, namelen );
        namelen--;  /* remove terminating null */
        entry = (const IMAGE_RESOURCE_DIRECTORY_ENTRY *)(dir + 1);
        min = 0;
        max = dir->NumberOfNamedEntries - 1;
        while (min <= max)
        {
            pos = (min + max) / 2;
            str = (const IMAGE_RESOURCE_DIRECTORY_STRING *)((const char *)root + entry[pos].u.s.NameOffset);
            res = _fstrnicmp( name, str->NameString, str->Length );
            if (!res && namelen == str->Length)
            {
                ret = (const IMAGE_RESOURCE_DIRECTORY *)((const char *)root + entry[pos].u2.s2.OffsetToDirectory);
                break;
            }
            if (res < 0) max = pos - 1;
            else min = pos + 1;
        }
//        HeapFree( GetProcessHeap(), 0, nameW );
    }
    return ret;
}


#pragma pack(push,2)
typedef struct _IMAGE_DOS_HEADER {
    WORD  e_magic;      /* 00: MZ Header signature */
    WORD  e_cblp;       /* 02: Bytes on last page of file */
    WORD  e_cp;         /* 04: Pages in file */
    WORD  e_crlc;       /* 06: Relocations */
    WORD  e_cparhdr;    /* 08: Size of header in paragraphs */
    WORD  e_minalloc;   /* 0a: Minimum extra paragraphs needed */
    WORD  e_maxalloc;   /* 0c: Maximum extra paragraphs needed */
    WORD  e_ss;         /* 0e: Initial (relative) SS value */
    WORD  e_sp;         /* 10: Initial SP value */
    WORD  e_csum;       /* 12: Checksum */
    WORD  e_ip;         /* 14: Initial IP value */
    WORD  e_cs;         /* 16: Initial (relative) CS value */
    WORD  e_lfarlc;     /* 18: File address of relocation table */
    WORD  e_ovno;       /* 1a: Overlay number */
    WORD  e_res[4];     /* 1c: Reserved words */
    WORD  e_oemid;      /* 24: OEM identifier (for e_oeminfo) */
    WORD  e_oeminfo;    /* 26: OEM information; e_oemid specific */
    WORD  e_res2[10];   /* 28: Reserved words */
    DWORD e_lfanew;     /* 3c: Offset to extended header */
} IMAGE_DOS_HEADER, *PIMAGE_DOS_HEADER;
#pragma pack(pop)

#define IMAGE_DOS_SIGNATURE    0x5A4D     /* MZ   */
#define IMAGE_OS2_SIGNATURE    0x454E     /* NE   */
#define IMAGE_OS2_SIGNATURE_LE 0x454C     /* LE   */
#define IMAGE_OS2_SIGNATURE_LX 0x584C     /* LX */
#define IMAGE_VXD_SIGNATURE    0x454C     /* LE   */
#define IMAGE_NT_SIGNATURE     0x00004550 /* PE00 */

#pragma pack(push,2)
typedef struct
{
    WORD  ne_magic;             /* 00 NE signature 'NE' */
    BYTE  ne_ver;               /* 02 Linker version number */
    BYTE  ne_rev;               /* 03 Linker revision number */
    WORD  ne_enttab;            /* 04 Offset to entry table relative to NE */
    WORD  ne_cbenttab;          /* 06 Length of entry table in bytes */
    LONG  ne_crc;               /* 08 Checksum */
    WORD  ne_flags;             /* 0c Flags about segments in this file */
    WORD  ne_autodata;          /* 0e Automatic data segment number */
    WORD  ne_heap;              /* 10 Initial size of local heap */
    WORD  ne_stack;             /* 12 Initial size of stack */
    DWORD ne_csip;              /* 14 Initial CS:IP */
    DWORD ne_sssp;              /* 18 Initial SS:SP */
    WORD  ne_cseg;              /* 1c # of entries in segment table */
    WORD  ne_cmod;              /* 1e # of entries in module reference tab. */
    WORD  ne_cbnrestab;         /* 20 Length of nonresident-name table     */
    WORD  ne_segtab;            /* 22 Offset to segment table */
    WORD  ne_rsrctab;           /* 24 Offset to resource table */
    WORD  ne_restab;            /* 26 Offset to resident-name table */
    WORD  ne_modtab;            /* 28 Offset to module reference table */
    WORD  ne_imptab;            /* 2a Offset to imported name table */
    DWORD ne_nrestab;           /* 2c Offset to nonresident-name table */
    WORD  ne_cmovent;           /* 30 # of movable entry points */
    WORD  ne_align;             /* 32 Logical sector alignment shift count */
    WORD  ne_cres;              /* 34 # of resource segments */
    BYTE  ne_exetyp;            /* 36 Flags indicating target OS */
    BYTE  ne_flagsothers;       /* 37 Additional information flags */
    WORD  ne_pretthunks;        /* 38 Offset to return thunks */
    WORD  ne_psegrefbytes;      /* 3a Offset to segment ref. bytes */
    WORD  ne_swaparea;          /* 3c Reserved by Microsoft */
    WORD  ne_expver;            /* 3e Expected Windows version number */
} IMAGE_OS2_HEADER, *PIMAGE_OS2_HEADER;
#pragma pack(pop)

#pragma pack(push,1)
typedef struct
{
    WORD     offset;
    WORD     length;
    WORD     flags;
    WORD     id;
    HANDLE   handle;
    WORD     usage;
} NE_NAMEINFO;

typedef struct
{
    WORD        type_id;   /* Type identifier */
    WORD        count;     /* Number of resources of this type */
    FARPROC     resloader; /* SetResourceHandler() */
    /*
     * Name info array.
     */
} NE_TYPEINFO;

#pragma pack(pop)

#define FILE_BEGIN      0L
#define FILE_CURRENT    1L
#define FILE_END        2L

/***********************************************************************
 *           read_xx_header         [internal]
 */
static int read_xx_header( HFILE lzfd )
{
    IMAGE_DOS_HEADER mzh;
    char magic[3];

    LZSeek( lzfd, 0, FILE_BEGIN );
    if ( sizeof(mzh) != LZRead( lzfd, (LPSTR)&mzh, sizeof(mzh) ) )
        return 0;
    if ( mzh.e_magic != IMAGE_DOS_SIGNATURE )
        return 0;

    LZSeek( lzfd, mzh.e_lfanew, FILE_BEGIN );
    if ( 2 != LZRead( lzfd, magic, 2 ) )
        return 0;

    LZSeek( lzfd, mzh.e_lfanew, FILE_BEGIN );

    if ( magic[0] == 'N' && magic[1] == 'E' )
        return IMAGE_OS2_SIGNATURE;
    if ( magic[0] == 'P' && magic[1] == 'E' )
        return IMAGE_NT_SIGNATURE;

    magic[2] = '\0';
//    WARN("Can't handle %s files.\n", magic );
    return 0;
}

/***********************************************************************
 *           find_ne_resource         [internal]
 */
static BOOL find_ne_resource( HFILE lzfd, LPCSTR typeid, LPCSTR resid,
                                DWORD *resLen, DWORD far *resOff )
{
    IMAGE_OS2_HEADER nehd;
    NE_TYPEINFO far *typeInfo;
    NE_NAMEINFO far *nameInfo;
    DWORD nehdoffset;
    LPBYTE resTab;
    DWORD resTabSize;
    int count;

    /* Read in NE header */
    nehdoffset = LZSeek( lzfd, 0, FILE_CURRENT );
    if ( sizeof(nehd) != LZRead( lzfd, (LPSTR)&nehd, sizeof(nehd) ) ) return FALSE;

    resTabSize = nehd.ne_restab - nehd.ne_rsrctab;
    if ( !resTabSize )
    {
//        TRACE("No resources in NE dll\n" );
        return FALSE;
    }

    /* Read in resource table */
    resTab = GlobalAllocPtr(GPTR, resTabSize );
    if ( !resTab ) return FALSE;

    LZSeek( lzfd, nehd.ne_rsrctab + nehdoffset, FILE_BEGIN );
    if ( resTabSize != LZRead( lzfd, (char*)resTab, resTabSize ) )
    {
        GlobalFreePtr( resTab );
        return FALSE;
    }

    /* Find resource */
    typeInfo = (NE_TYPEINFO far *)(resTab + 2);

    if (HIWORD(typeid) != 0)  /* named type */
    {
        BYTE len = lstrlen( typeid );
        while (typeInfo->type_id)
        {
            if (!(typeInfo->type_id & 0x8000))
            {
                BYTE far *p = resTab + typeInfo->type_id;
                if ((*p == len) && !_fstrnicmp( (char far *)p+1, typeid, len )) goto found_type;
            }
            typeInfo = (NE_TYPEINFO far*)((char far*)(typeInfo + 1) +
                                       typeInfo->count * sizeof(NE_NAMEINFO));
        }
    }
    else  /* numeric type id */
    {
        WORD id = LOWORD(typeid) | 0x8000;
        while (typeInfo->type_id)
        {
            if (typeInfo->type_id == id) goto found_type;
            typeInfo = (NE_TYPEINFO far *)((char far *)(typeInfo + 1) +
                                       typeInfo->count * sizeof(NE_NAMEINFO));
        }
    }
//    TRACE("No typeid entry found for %p\n", typeid );
    GlobalFreePtr( resTab );
    return FALSE;

 found_type:
    nameInfo = (NE_NAMEINFO far *)(typeInfo + 1);

    if (HIWORD(resid) != 0)  /* named resource */
    {
        BYTE len = lstrlen( resid );
        for (count = typeInfo->count; count > 0; count--, nameInfo++)
        {
            BYTE far *p = resTab + nameInfo->id;
            if (nameInfo->id & 0x8000) continue;
            if ((*p == len) && !_fstrnicmp( (char far *)p+1, resid, len )) goto found_name;
        }
    }
    else  /* numeric resource id */
    {
        WORD id = LOWORD(resid) | 0x8000;
        for (count = typeInfo->count; count > 0; count--, nameInfo++)
            if (nameInfo->id == id) goto found_name;
    }
//    TRACE("No resid entry found for %p\n", typeid );
    GlobalFreePtr( resTab );
    return FALSE;

 found_name:
    /* Return resource data */
    if ( resLen ) *resLen = nameInfo->length << *(WORD far *)resTab;
    if ( resOff ) *resOff = nameInfo->offset << *(WORD far *)resTab;

    GlobalFreePtr( resTab );
    return TRUE;
}

/***********************************************************************
 *           find_pe_resource         [internal]
 */
static BOOL find_pe_resource( HFILE lzfd, LPCSTR typeid, LPCSTR resid,
                                DWORD *resLen, DWORD far *resOff )
{
    IMAGE_NT_HEADERS pehd;
    DWORD pehdoffset;
    PIMAGE_DATA_DIRECTORY resDataDir;
    PIMAGE_SECTION_HEADER sections;
    LPBYTE resSection;
    DWORD resSectionSize;
    const void far *resDir;
    const IMAGE_RESOURCE_DIRECTORY far *resPtr;
    const IMAGE_RESOURCE_DATA_ENTRY *resData;
    int i, nSections;
    BOOL ret = FALSE;

    /* Read in PE header */
    pehdoffset = LZSeek( lzfd, 0, FILE_CURRENT );
    if ( sizeof(pehd) != LZRead( lzfd, (LPSTR)&pehd, sizeof(pehd) ) ) return FALSE;

    resDataDir = pehd.OptionalHeader.DataDirectory+IMAGE_DIRECTORY_ENTRY_RESOURCE;
    if ( !resDataDir->Size )
    {
//        TRACE("No resources in PE dll\n" );
        return FALSE;
    }

    /* Read in section table */
    nSections = pehd.FileHeader.NumberOfSections;
    sections = (PIMAGE_SECTION_HEADER)GlobalAllocPtr( GPTR,
                          nSections * sizeof(IMAGE_SECTION_HEADER) );
    if ( !sections ) return FALSE;

    LZSeek( lzfd, pehdoffset +
                    sizeof(DWORD) + /* Signature */
                    sizeof(IMAGE_FILE_HEADER) +
                    pehd.FileHeader.SizeOfOptionalHeader, FILE_BEGIN );

    if ( nSections * sizeof(IMAGE_SECTION_HEADER) !=
         LZRead( lzfd, (LPSTR)sections, nSections * sizeof(IMAGE_SECTION_HEADER) ) )
    {
        GlobalFreePtr( sections );
        return FALSE;
    }

    /* Find resource section */
    for ( i = 0; i < nSections; i++ )
        if (    resDataDir->VirtualAddress >= sections[i].VirtualAddress
             && resDataDir->VirtualAddress <  sections[i].VirtualAddress +
                                              sections[i].SizeOfRawData )
            break;

    if ( i == nSections )
    {
        GlobalFreePtr( sections );
//        TRACE("Couldn't find resource section\n" );
        return FALSE;
    }

    /* Read in resource section */
    resSectionSize = sections[i].SizeOfRawData;
    resSection = GlobalAllocPtr( GPTR, resSectionSize );
    if ( !resSection )
    {
        GlobalFreePtr( sections );
        return FALSE;
    }

    LZSeek( lzfd, sections[i].PointerToRawData, FILE_BEGIN );
    if ( resSectionSize != LZRead( lzfd, (char*)resSection, resSectionSize ) ) goto done;

    /* Find resource */
    resDir = resSection + (resDataDir->VirtualAddress - sections[i].VirtualAddress);

    resPtr = resDir;
    resPtr = find_entry_by_name( resPtr, typeid, resDir );
    if ( !resPtr )
    {
//        TRACE("No typeid entry found for %p\n", typeid );
        goto done;
    }
    resPtr = find_entry_by_name( resPtr, resid, resDir );
    if ( !resPtr )
    {
//        TRACE("No resid entry found for %p\n", resid );
        goto done;
    }
    resPtr = find_entry_default( resPtr, resDir );
    if ( !resPtr )
    {
//        TRACE("No default language entry found for %p\n", resid );
        goto done;
    }

    /* Find resource data section */
    resData = (const IMAGE_RESOURCE_DATA_ENTRY*)resPtr;
    for ( i = 0; i < nSections; i++ )
        if (    resData->OffsetToData >= sections[i].VirtualAddress
             && resData->OffsetToData <  sections[i].VirtualAddress +
                                         sections[i].SizeOfRawData )
            break;

    if ( i == nSections )
    {
//        TRACE("Couldn't find resource data section\n" );
        goto done;
    }

    /* Return resource data */
    if ( resLen ) *resLen = resData->Size;
    if ( resOff ) *resOff = resData->OffsetToData - sections[i].VirtualAddress
                            + sections[i].PointerToRawData;
    ret = TRUE;

 done:
    GlobalFreePtr( resSection );
    GlobalFreePtr( sections );
    return ret;
}


/***********************************************************************
 *           find_resource         [internal]
 */
static DWORD find_resource( HFILE lzfd, LPCSTR type, LPCSTR id, DWORD *reslen, DWORD far *offset )
{
    DWORD magic = read_xx_header( lzfd );

    switch (magic)
    {
    case IMAGE_OS2_SIGNATURE:
        if (!find_ne_resource( lzfd, type, id, reslen, offset )) magic = 0;
        break;
    case IMAGE_NT_SIGNATURE:
        if (!find_pe_resource( lzfd, type, id, reslen, offset )) magic = 0;
        break;
    }
    return magic;
}


/*************************************************************************
 * GetFileResourceSize                     [VER.2]
 */
DWORD WINAPI GetFileResourceSize( LPCSTR lpszFileName, LPCSTR lpszResType,
                                    LPCSTR lpszResId, LPDWORD lpdwFileOffset )
{
    HFILE lzfd;
    OFSTRUCT ofs;
    DWORD reslen = 0;

//    TRACE("(%s,type=%p,id=%p,off=%p)\n",
//          debugstr_a(lpszFileName), lpszResType, lpszResId, lpszResId );

    lzfd = LZOpenFile( (LPSTR)lpszFileName, &ofs, OF_READ );
    if (lzfd >= 0)
    {
        if (!find_resource( lzfd, lpszResType, lpszResId, &reslen, lpdwFileOffset )) reslen = 0;
        LZClose( lzfd );
    }
    return reslen;
}


/*************************************************************************
 * GetFileResource                         [VER.3]
 */
int WINAPI GetFileResource( LPCSTR lpszFileName, LPCSTR lpszResType,
                                LPCSTR lpszResId, DWORD dwFileOffset,
                                DWORD dwResLen, LPVOID lpvData )
{
    HFILE lzfd;
    OFSTRUCT ofs;
    DWORD reslen = dwResLen;

//    TRACE("(%s,type=%p,id=%p,off=%ld,len=%ld,data=%p)\n",
//		debugstr_a(lpszFileName), lpszResType, lpszResId,
//                dwFileOffset, dwResLen, lpvData );

    lzfd = LZOpenFile( (LPSTR)lpszFileName, &ofs, OF_READ );
    if ( lzfd < 0 ) return 0;

    if ( !dwFileOffset )
    {
        if (!find_resource( lzfd, lpszResType, lpszResId, &reslen, &dwFileOffset ))
        {
            LZClose( lzfd );
            return 0;
        }
    }

    LZSeek( lzfd, dwFileOffset, FILE_BEGIN );
    reslen = LZRead( lzfd, lpvData, min( reslen, dwResLen ) );
    LZClose( lzfd );

    return reslen;
}

/*************************************************************************
 * GetFileVersionInfoSize                  [VER.6]
 */
DWORD WINAPI GetFileVersionInfoSize( LPCSTR filename, LPDWORD handle )
{
    DWORD offset;

//    TRACE("(%s, %p)\n", debugstr_a(filename), handle );

    return GetFileResourceSize( filename, (LPCSTR)RT_VERSION, (LPCSTR)VS_VERSION_INFO, &offset );
}

/*************************************************************************
 * GetFileVersionInfo                      [VER.7]
 */
int WINAPI GetFileVersionInfo( LPCSTR filename, DWORD handle, DWORD datasize, LPVOID data )
{
//    TRACE("(%s, %08lx, %ld, %p)\n", debugstr_a(filename), handle, datasize, data );

    return GetFileResource( filename, (LPCSTR)RT_VERSION, (LPCSTR)VS_VERSION_INFO, 0, datasize, data );
}

/******************************************************************************
 *   testFileExistenceA
 *
 *   Tests whether a given path/file combination exists.  If the file does
 *   not exist, the return value is zero.  If it does exist, the return
 *   value is non-zero.
 *
 *   Revision history
 *      30-May-1997 Dave Cuthbert (dacut@ece.cmu.edu)
 *         Original implementation
 *
 */
static int testFileExistenceA( char const far * path, char const far * file, BOOL excl )
{
    char  filename[1024];
    int  filenamelen;
    OFSTRUCT  fileinfo;

    fileinfo.cBytes = sizeof(OFSTRUCT);

    lstrcpy(filename, path);
    filenamelen = lstrlen(filename);

    /* Add a trailing \ if necessary */
    if(filenamelen) {
	if(filename[filenamelen - 1] != '\\')
	    lstrcat(filename, "\\");
    }
    else /* specify the current directory */
		lstrcpy(filename, ".\\");

    /* Create the full pathname */
    lstrcat(filename, file);

    return (OpenFile(filename, &fileinfo,
                     OF_EXIST | (excl ? OF_SHARE_EXCLUSIVE : 0)) != HFILE_ERROR);
}

/*****************************************************************************
 *   VerFindFileA [internal]
 *
 *   Determines where to install a file based on whether it locates another
 *   version of the file in the system.  The values VerFindFile returns are
 *   used in a subsequent call to the VerInstallFile function.
 *
 *   Revision history:
 *      30-May-1997   Dave Cuthbert (dacut@ece.cmu.edu)
 *         Reimplementation of VerFindFile from original stub.
 */
DWORD WINAPI VerFindFileA(
    UINT flags,
    LPCSTR lpszFilename,
    LPCSTR lpszWinDir,
    LPCSTR lpszAppDir,
    LPSTR lpszCurDir,
    UINT *lpuCurDirLen,
    LPSTR lpszDestDir,
    UINT *lpuDestDirLen )
{
    DWORD  retval = 0;
    const char far *curDir;
    const char far *destDir;
    unsigned int  curDirSizeReq;
    unsigned int  destDirSizeReq;
    char  systemDir[260];  // @todo use some constant here

    /* Print out debugging information */
/*    TRACE("flags = %x filename=%s windir=%s appdir=%s curdirlen=%p(%u) destdirlen=%p(%u)\n",
          flags, debugstr_a(lpszFilename), debugstr_a(lpszWinDir), debugstr_a(lpszAppDir),
          lpuCurDirLen, lpuCurDirLen ? *lpuCurDirLen : 0,
          lpuDestDirLen, lpuDestDirLen ? *lpuDestDirLen : 0 );
*/
    /* Figure out where the file should go; shared files default to the
       system directory */

    GetSystemDirectory(systemDir, sizeof(systemDir));
    curDir = "";
    destDir = "";

    if(flags & VFFF_ISSHAREDFILE)
    {
        destDir = systemDir;
        /* Were we given a filename?  If so, try to find the file. */
        if(lpszFilename)
        {
            if(testFileExistenceA(destDir, lpszFilename, FALSE)) curDir = destDir;
            else if(lpszAppDir && testFileExistenceA(lpszAppDir, lpszFilename, FALSE))
            {
                curDir = lpszAppDir;
                retval |= VFF_CURNEDEST;
            }
        }
    }
    else /* not a shared file */
    {
        if(lpszAppDir)
        {
            destDir = lpszAppDir;
            if(lpszFilename)
            {
                if(testFileExistenceA(destDir, lpszFilename, FALSE)) curDir = destDir;
                else if(testFileExistenceA(systemDir, lpszFilename, FALSE))
                {
                    curDir = systemDir;
                    retval |= VFF_CURNEDEST;
                }
            }
        }
    }

    if (lpszFilename && !testFileExistenceA(curDir, lpszFilename, TRUE))
        retval |= VFF_FILEINUSE;

    curDirSizeReq = lstrlen(curDir) + 1;
    destDirSizeReq = lstrlen(destDir) + 1;

    /* Make sure that the pointers to the size of the buffers are
       valid; if not, do NOTHING with that buffer.  If that pointer
       is valid, then make sure that the buffer pointer is valid, too! */

    if(lpuDestDirLen && lpszDestDir)
    {
        if (*lpuDestDirLen < destDirSizeReq) retval |= VFF_BUFFTOOSMALL;
        lstrcpyn(lpszDestDir, destDir, *lpuDestDirLen);
        *lpuDestDirLen = destDirSizeReq;
    }
    if(lpuCurDirLen && lpszCurDir)
    {
        if(*lpuCurDirLen < curDirSizeReq) retval |= VFF_BUFFTOOSMALL;
        lstrcpyn(lpszCurDir, curDir, *lpuCurDirLen);
        *lpuCurDirLen = curDirSizeReq;
    }

/*
    TRACE("ret = %lu (%s%s%s) curdir=%s destdir=%s\n", retval,
          (retval & VFF_CURNEDEST) ? "VFF_CURNEDEST " : "",
          (retval & VFF_FILEINUSE) ? "VFF_FILEINUSE " : "",
          (retval & VFF_BUFFTOOSMALL) ? "VFF_BUFFTOOSMALL " : "",
          debugstr_a(lpszCurDir), debugstr_a(lpszDestDir));
*/
    return retval;
}

/*************************************************************************
 * VerFindFile                             [VER.8]
 */
UINT WINAPI VerFindFile( UINT flags, LPCSTR lpszFilename,
                            LPCSTR lpszWinDir, LPCSTR lpszAppDir,
                            LPSTR lpszCurDir, UINT far *lpuCurDirLen,
                            LPSTR lpszDestDir, UINT far *lpuDestDirLen )
{
    UINT curDirLen, destDirLen;
    UINT *pcurDirLen = NULL, *pdestDirLen = NULL;
    DWORD retv;

    if (lpuCurDirLen) {
        curDirLen = *lpuCurDirLen;
        pcurDirLen = &curDirLen;
    }
    if (lpuDestDirLen) {
        destDirLen = *lpuDestDirLen;
        pdestDirLen = &destDirLen;
    }
    retv = VerFindFileA( flags, lpszFilename, lpszWinDir, lpszAppDir,
                                lpszCurDir, pcurDirLen, lpszDestDir, pdestDirLen );
    if (lpuCurDirLen)
        *lpuCurDirLen = (UINT)curDirLen;
    if (lpuDestDirLen)
        *lpuDestDirLen = (UINT)destDirLen;
    return retv;
}

#define DWORD_ALIGN( base, ptr ) \
    ( (LPBYTE)(base) + ((((LPBYTE)(ptr) - (LPBYTE)(base)) + 3) & ~3) )

#define VersionInfo16_Value( ver )  \
    DWORD_ALIGN( (ver), (ver)->szKey + strlen((ver)->szKey) + 1 )
#define VersionInfo16_Next( ver ) \
    (VS_VERSION_INFO_STRUCT16 *)( (LPBYTE)ver + (((ver)->wLength + 3) & ~3) )

#define VersionInfo16_Value( ver )  \
    DWORD_ALIGN( (ver), (ver)->szKey + strlen((ver)->szKey) + 1 )

#define VersionInfo16_Children( ver )  \
    (const VS_VERSION_INFO_STRUCT16 far *)( VersionInfo16_Value( ver ) + \
                           ( ( (ver)->wValueLength + 3 ) & ~3 ) )

/***********************************************************************
 *           VersionInfo16_FindChild             [internal]
 */
static const VS_VERSION_INFO_STRUCT16 far *VersionInfo16_FindChild( const VS_VERSION_INFO_STRUCT16 far *info,
                                                                LPCSTR key, UINT len )
{
    const VS_VERSION_INFO_STRUCT16 far *child = VersionInfo16_Children( info );

    while ((char far *)child < (char far *)info + info->wLength )
    {
        if (!_fstrnicmp( child->szKey, key, len ) && !child->szKey[len])
            return child;

        if (!(child->wLength)) return NULL;
        child = VersionInfo16_Next( child );
    }

    return NULL;
}

/***********************************************************************
 *           VersionInfo16_QueryValue              [internal]
 *
 *    Gets a value from a 16-bit NE resource
 */
static BOOL VersionInfo16_QueryValue( const VS_VERSION_INFO_STRUCT16 far *info, LPCSTR lpSubBlock,
                               LPVOID *lplpBuffer, UINT far *puLen )
{
    while ( *lpSubBlock )
    {
        /* Find next path component */
        LPCSTR lpNextSlash;
        for ( lpNextSlash = lpSubBlock; *lpNextSlash; lpNextSlash++ )
            if ( *lpNextSlash == '\\' )
                break;

        /* Skip empty components */
        if ( lpNextSlash == lpSubBlock )
        {
            lpSubBlock++;
            continue;
        }

        /* We have a non-empty component: search info for key */
        info = VersionInfo16_FindChild( info, lpSubBlock, lpNextSlash-lpSubBlock );
        if ( !info )
        {
            if (puLen) *puLen = 0 ;
//            SetLastError( ERROR_RESOURCE_TYPE_NOT_FOUND );
            return FALSE;
        }

        /* Skip path component */
        lpSubBlock = lpNextSlash;
    }

    /* Return value */
    *lplpBuffer = VersionInfo16_Value( info );
    if (puLen)
        *puLen = info->wValueLength;

    return TRUE;
}


/***********************************************************************
 *           VerQueryValueA              [internal]
 */
BOOL WINAPI VerQueryValueA( void const far * pBlock, LPCSTR lpSubBlock,
                               LPVOID *lplpBuffer, UINT far * puLen )
{
    static const char rootA[] = "\\";
    const VS_VERSION_INFO_STRUCT16 far *info = pBlock;

//    TRACE("(%p,%s,%p,%p)\n",
//                pBlock, debugstr_a(lpSubBlock), lplpBuffer, puLen );

     if (!pBlock)
        return FALSE;

    if (lpSubBlock == NULL || lpSubBlock[0] == '\0')
        lpSubBlock = rootA;

    return VersionInfo16_QueryValue(info, lpSubBlock, lplpBuffer, puLen);
}

/*************************************************************************
 * VerQueryValue                          [VER.11]
 */
int WINAPI VerQueryValue( void const far * spvBlock, LPCSTR lpszSubBlock,
                              void far * far *lpspBuffer, UINT far  *lpcb )
{
    LPVOID lpvBlock = (LPVOID)spvBlock;
    LPVOID buffer = lpvBlock;
    UINT buflen;
    DWORD retv;

//    TRACE("(%p, %s, %p, %p)\n",
//                lpvBlock, debugstr_a(lpszSubBlock), lpspBuffer, lpcb );

    retv = VerQueryValueA( lpvBlock, lpszSubBlock, &buffer, &buflen );
    if ( !retv ) return FALSE;

    if ( OFFSETOF( spvBlock ) + ((char far *) buffer - (char far *) lpvBlock) >= 0x10000 )
    {
//        FIXME("offset %08X too large relative to %04X:%04X\n",
//               (char *) buffer - (char *) lpvBlock, SELECTOROF( spvBlock ), OFFSETOF( spvBlock ) );
        return FALSE;
    }

    if (lpcb) *lpcb = buflen;
    *lpspBuffer = (void far *) ((char far *) spvBlock + ((char far *) buffer - (char far *) lpvBlock));

    return retv;
}


static LPBYTE
_fetch_versioninfo(LPSTR fn,VS_FIXEDFILEINFO **vffi) {
    DWORD	alloclen;
    LPBYTE	buf;
    DWORD	ret;

    alloclen = 1000;
    buf=GlobalAllocPtr(GPTR, alloclen);
    if(buf == NULL) {
        //WARN("Memory exausted while fetching version info!\n");
        return NULL;
    }
    while (1) {
    	ret = GetFileVersionInfo(fn,0,alloclen,buf);
	if (!ret) {
	    GlobalFreePtr(buf);
	    return NULL;
	}
	if (alloclen<*(WORD*)buf) {
	    alloclen = *(WORD*)buf;
	    GlobalFreePtr(buf);
	    buf = GlobalAllocPtr(GPTR, alloclen);
            if(buf == NULL) {
          //     WARN("Memory exausted while fetching version info!\n");
               return NULL;
            }
	} else {
	    *vffi = (VS_FIXEDFILEINFO*)(buf+0x14);
	    if ((*vffi)->dwSignature == 0x004f0049) /* hack to detect unicode */
	    	*vffi = (VS_FIXEDFILEINFO*)(buf+0x28);
	    if ((*vffi)->dwSignature != VS_FFI_SIGNATURE)
		{
	    //	WARN("Bad VS_FIXEDFILEINFO signature 0x%08lx\n",(*vffi)->dwSignature);
		}
	    return buf;
	}
    }
}

/******************************************************************************
 * VerInstallFileA [internal]
 */
DWORD WINAPI VerInstallFileA(
	UINT flags,LPCSTR srcfilename,LPCSTR destfilename,LPCSTR srcdir,
 	LPCSTR destdir,LPCSTR curdir,LPSTR tmpfile,UINT *tmpfilelen )
{
    LPCSTR pdest;
    char	destfn[260],tmpfn[260],srcfn[260];
    HFILE	hfsrc,hfdst;
    unsigned    attr;
    DWORD	ret,xret,tmplast;
    LPBYTE	buf1,buf2;
    OFSTRUCT	ofs;

//    TRACE("(%x,%s,%s,%s,%s,%s,%p,%d)\n",
//	    flags,srcfilename,destfilename,srcdir,destdir,curdir,tmpfile,*tmpfilelen
//    );
    xret = 0;
    sprintf(srcfn,"%s\\%s",srcdir,srcfilename);
    if (!destdir || !*destdir) pdest = srcdir;
    else pdest = destdir;
    sprintf(destfn,"%s\\%s",pdest,destfilename);
    hfsrc=LZOpenFile(srcfn,&ofs,OF_READ);
    if (hfsrc < 0)
    	return VIF_CANNOTREADSRC;
    sprintf(tmpfn,"%s\\%s",pdest,destfilename);
    tmplast=lstrlen(pdest)+1;
//    attr = GetFileAttributes(tmpfn);
    if (!_dos_getfileattr(tmpfn, &attr)) {
//	if (attr & FILE_ATTRIBUTE_READONLY) {
        if (attr & _A_RDONLY	) {
	    LZClose(hfsrc);
	    return VIF_WRITEPROT;
	}
	/* FIXME: check if file currently in use and return VIF_FILEINUSE */
    }
    attr = INVALID_FILE_ATTRIBUTES;
    if (flags & VIFF_FORCEINSTALL) {
    	if (tmpfile[0]) {
	    sprintf(tmpfn,"%s\\%s",pdest,tmpfile);
	    tmplast = lstrlen(pdest)+1;
//	    attr = GetFileAttributes(tmpfn);
            if (_dos_getfileattr(tmpfn, &attr)) 
            {
              attr = INVALID_FILE_ATTRIBUTES;
            }
	    /* if it exists, it has been copied by the call before.
	     * we jump over the copy part...
	     */
	}
    }
    if (attr == INVALID_FILE_ATTRIBUTES) {
    	char far *s;

	GetTempFileName(*pdest,"ver",0,tmpfn); /* should not fail ... */
	s=_fstrrchr(tmpfn,'\\');
	if (s)
	    tmplast = s-tmpfn;
	else
	    tmplast = 0;
	hfdst = OpenFile(tmpfn,&ofs,OF_CREATE);
	if (hfdst == HFILE_ERROR) {
	    LZClose(hfsrc);
	    return VIF_CANNOTCREATE; /* | translated dos error */
	}
	ret = LZCopy(hfsrc,hfdst);
	_lclose(hfdst);
	if (((long) ret) < 0) {
	    /* translate LZ errors into VIF_xxx */
	    switch (ret) {
	    case LZERROR_BADINHANDLE:
	    case LZERROR_READ:
	    case LZERROR_BADVALUE:
	    case LZERROR_UNKNOWNALG:
		ret = VIF_CANNOTREADSRC;
		break;
	    case LZERROR_BADOUTHANDLE:
	    case LZERROR_WRITE:
		ret = VIF_OUTOFSPACE;
		break;
	    case LZERROR_GLOBALLOC:
	    case LZERROR_GLOBLOCK:
		ret = VIF_OUTOFMEMORY;
		break;
	    default: /* unknown error, should not happen */
		ret = 0;
		break;
	    }
	    if (ret) {
		LZClose(hfsrc);
		return ret;
	    }
	}
    }
    xret = 0;
    if (!(flags & VIFF_FORCEINSTALL)) {
	VS_FIXEDFILEINFO *destvffi,*tmpvffi;
    	buf1 = _fetch_versioninfo(destfn,&destvffi);
	if (buf1) {
	    buf2 = _fetch_versioninfo(tmpfn,&tmpvffi);
	    if (buf2) {
	    	char	*tbuf1,*tbuf2;
		UINT	len1,len2;

		len1=len2=40;

		/* compare file versions */
		if ((destvffi->dwFileVersionMS > tmpvffi->dwFileVersionMS)||
		    ((destvffi->dwFileVersionMS==tmpvffi->dwFileVersionMS)&&
		     (destvffi->dwFileVersionLS > tmpvffi->dwFileVersionLS)
		    )
		)
		    xret |= VIF_MISMATCH|VIF_SRCOLD;
		/* compare filetypes and filesubtypes */
		if ((destvffi->dwFileType!=tmpvffi->dwFileType) ||
		    (destvffi->dwFileSubtype!=tmpvffi->dwFileSubtype)
		)
		    xret |= VIF_MISMATCH|VIF_DIFFTYPE;
		if (VerQueryValueA(buf1,"\\VarFileInfo\\Translation",(LPVOID*)&tbuf1,&len1) &&
		    VerQueryValueA(buf2,"\\VarFileInfo\\Translation",(LPVOID*)&tbuf2,&len2)
		) {
		    /* irgendwas mit tbuf1 und tbuf2 machen
		     * generiert DIFFLANG|MISMATCH
		     */
		}
		GlobalFreePtr(buf2);
	    } else
		xret=VIF_MISMATCH|VIF_SRCOLD;
	    GlobalFreePtr(buf1);
	}
    }
    if (xret) {
	if (*tmpfilelen<lstrlen(tmpfn+tmplast)) {
	    xret|=VIF_BUFFTOOSMALL;
	    remove(tmpfn);
	} else {
	    lstrcpy(tmpfile,tmpfn+tmplast);
	    *tmpfilelen = lstrlen(tmpfn+tmplast)+1;
	    xret|=VIF_TEMPFILE;
	}
    } else {
    	if (!_dos_getfileattr(destfn, &attr))
	    if (!remove(destfn)) {
		xret|=/*_error2vif(GetLastError())|*/VIF_CANNOTDELETE;
		remove(tmpfn);
		LZClose(hfsrc);
		return xret;
	    }
	if ((!(flags & VIFF_DONTDELETEOLD))	&&
	    curdir				&&
	    *curdir				&&
	    lstrcmpi(curdir,pdest)
	) {
	    char curfn[260];

	    sprintf(curfn,"%s\\%s",curdir,destfilename);
	    if (!_dos_getfileattr(curfn, &attr)) {
		/* FIXME: check if in use ... if it is, VIF_CANNOTDELETECUR */
		if (!remove(curfn))
	    	    xret|=/*_error2vif(GetLastError())|*/VIF_CANNOTDELETECUR;
	    }
	}
	if (!MoveFile(tmpfn,destfn)) {
	    xret|=/*_error2vif(GetLastError())|*/VIF_CANNOTRENAME;
	    remove(tmpfn);
	}
    }
    LZClose(hfsrc);
    return xret;
}

/*************************************************************************
 * VerInstallFile                          [VER.9]
 */
DWORD WINAPI VerInstallFile( UINT flags,
                               LPCSTR lpszSrcFilename, LPCSTR lpszDestFilename,
                               LPCSTR lpszSrcDir, LPCSTR lpszDestDir, LPCSTR lpszCurDir,
                               LPSTR lpszTmpFile, UINT far *lpwTmpFileLen )
{
    UINT filelen = *lpwTmpFileLen;

    DWORD retv = VerInstallFileA( flags, lpszSrcFilename, lpszDestFilename,
                                    lpszSrcDir, lpszDestDir, lpszCurDir,
                                    lpszTmpFile, &filelen);

    *lpwTmpFileLen = (UINT)filelen;
    return retv;
}


/*************************************************************************
 * VerLanguageName                        [VER.10]
 */
UINT WINAPI VerLanguageName( UINT uLang, LPSTR lpszLang, UINT cbLang )
{
    // in Windows 3.x country information stored in control.inf file
    return 0;
}

BOOL FAR PASCAL LibMain( HINSTANCE hInstance, WORD wDataSegment,
                         WORD wHeapSize, LPSTR lpszCmdLine )
{
  return( 1 );
}
