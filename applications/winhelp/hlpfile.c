/*
 * Help Viewer
 *
 * Copyright    1996 Ulrich Schmid
 *              2002 Eric Pouech
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
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#include <stdarg.h>
#include <stdio.h>
#include <string.h>

//#include "windef.h"
//#include "winbase.h"
//#include "wingdi.h"
//#include "winuser.h"
#include <windows.h>

#define GlobalPtrHandle(lp) \
  ((HGLOBAL)LOWORD(GlobalHandle(SELECTOROF(lp))))

#define     GlobalUnlockPtr(lp)      \
                GlobalUnlock(GlobalPtrHandle(lp))

#define GlobalFreePtr(lp) \
  (GlobalUnlockPtr(lp),(BOOL)GlobalFree(GlobalPtrHandle(lp)))

#define GlobalAllocPtr(flags, cb) \
  (GlobalLock(GlobalAlloc((flags), (cb))))


#include "winhelp.h"

#include <hlpfmt.h>

//#include "wine/debug.h"

//WINE_DEFAULT_DEBUG_CHANNEL(winhelp);

static inline unsigned short GET_USHORT(const BYTE far * buffer, unsigned i)
{
    return (BYTE)buffer[i] + 0x100 * (BYTE)buffer[i + 1];
}

static inline short GET_SHORT(const BYTE far * buffer, unsigned i)
{
    return (BYTE)buffer[i] + 0x100 * (signed char)buffer[i+1];
}

static inline unsigned long GET_ULONG(const BYTE far * buffer, unsigned i)
{
    return GET_USHORT(buffer, i) + 0x10000 * GET_USHORT(buffer, i + 2);
}

static HLPFILE far *first_hlpfile = 0;
static BYTE far *file_buffer;

static struct
{
    UINT        num;
    unsigned far *   offsets;
    LPSTR       buffer;
} phrases;

static struct
{
    BYTE far * far * map;
    BYTE far *       end;
    UINT        wMapLen;
} topic;

static struct
{
    UINT                wFont;
    UINT                wIndent;
    UINT                wHSpace;
    UINT                wVSpace;
    HLPFILE_LINK far *  link;
} attributes;

static BOOL  HLPFILE_DoReadHlpFile(HLPFILE far *, LPCSTR);
static BOOL  HLPFILE_ReadFileToBuffer(HFILE);
static BOOL  HLPFILE_FindSubFile(LPCSTR name, BYTE far * far *, BYTE far * far *);
static BOOL  HLPFILE_SystemCommands(HLPFILE far *);
static int   HLPFILE_UncompressedLZ77_Size(BYTE far * ptr, BYTE far * end);
static BYTE far * HLPFILE_UncompressLZ77(BYTE far * ptr, BYTE far * end, BYTE far *newptr);
static BOOL  HLPFILE_UncompressLZ77_Phrases(HLPFILE far *);
static BOOL  HLPFILE_Uncompress_Phrases40(HLPFILE far *);
static BOOL  HLPFILE_Uncompress_Topic(HLPFILE far *);
static BOOL  HLPFILE_GetContext(HLPFILE far *);
static BOOL  HLPFILE_AddPage(HLPFILE far *, BYTE far *, BYTE far *, unsigned);
static BOOL  HLPFILE_AddParagraph(HLPFILE far *, BYTE far *, BYTE far *, unsigned*);
static void  HLPFILE_Uncompress2(const BYTE far *, const BYTE far *, BYTE far *, const BYTE far *);
static BOOL  HLPFILE_Uncompress3(char far *, const char far *, const BYTE far *, const BYTE far *);
static void  HLPFILE_UncompressRLE(const BYTE far * src, const BYTE far * end, BYTE far * far * dst, unsigned dstsz);
static BOOL  HLPFILE_ReadFont(HLPFILE far * hlpfile);

#if 0
/***********************************************************************
 *
 *           HLPFILE_PageByNumber
 */
static HLPFILE_PAGE *HLPFILE_PageByNumber(LPCSTR lpszPath, UINT wNum)
{
    HLPFILE_PAGE *page;
    HLPFILE *hlpfile = HLPFILE_ReadHlpFile(lpszPath);

    if (!hlpfile) return 0;

    //WINE_TRACE("[%s/%u]\n", lpszPath, wNum);

    for (page = hlpfile->first_page; page && wNum; page = page->next) wNum--;

    /* HLPFILE_FreeHlpFile(lpszPath); */

    return page;
}
#endif

/* FIXME:
 * this finds the page containing the offset. The offset can either
 * refer to the top of the page (offset == page->offset), or
 * to some paragraph inside the page...
 * As of today, we only return the page... we should also return
 * a paragraph, and then, while opening a new page, compute the
 * y-offset of the paragraph to be shown and scroll the window
 * accordinly
 */
/******************************************************************
 *		HLPFILE_PageByOffset
 *
 *
 */
HLPFILE_PAGE far *HLPFILE_PageByOffset(HLPFILE far * hlpfile, LONG offset)
{
    HLPFILE_PAGE far *       page;
    HLPFILE_PAGE far *       found;

    if (!hlpfile) return 0;

    //WINE_TRACE("<%s>[%lx]\n", hlpfile->lpszPath, offset);

    if (offset == 0xFFFFFFFF) return NULL;
    page = NULL;

    for (found = NULL, page = hlpfile->first_page; page; page = page->next)
    {
        if (page->offset <= offset && (!found || found->offset < page->offset))
            found = page;
    }
    if (!found)
	{}//WINE_ERR("Page of offset %lu not found in file %s\n",
        //         offset, hlpfile->lpszPath);
    return found;
}

/***********************************************************************
 *
 *           HLPFILE_HlpFilePageByHash
 */
HLPFILE_PAGE far *HLPFILE_PageByHash(HLPFILE far * hlpfile, LONG lHash)
{
    int                 i;

    if (!hlpfile) return 0;

    //WINE_TRACE("<%s>[%lx]\n", hlpfile->lpszPath, lHash);

    for (i = 0; i < hlpfile->wContextLen; i++)
    {
        if (hlpfile->Context[i].lHash == lHash)
            return HLPFILE_PageByOffset(hlpfile, hlpfile->Context[i].offset);
    }

    //WINE_ERR("Page of hash %lx not found in file %s\n", lHash, hlpfile->lpszPath);
    return NULL;
}

/***********************************************************************
 *
 *           HLPFILE_Contents
 */
HLPFILE_PAGE far * HLPFILE_Contents(HLPFILE far *hlpfile)
{
    HLPFILE_PAGE far *       page = NULL;

    if (!hlpfile) return NULL;

    page = HLPFILE_PageByOffset(hlpfile, hlpfile->contents_start);
    if (!page) page = hlpfile->first_page;
    return page;
}

/***********************************************************************
 *
 *           HLPFILE_Hash
 */
LONG HLPFILE_Hash(LPCSTR lpszContext)
{
    LONG lHash = 0;
    char c;

    while ((c = *lpszContext++))
    {
        char x = 0;
        if (c >= 'A' && c <= 'Z') x = c - 'A' + 17;
        if (c >= 'a' && c <= 'z') x = c - 'a' + 17;
        if (c >= '1' && c <= '9') x = c - '0';
        if (c == '0') x = 10;
        if (c == '.') x = 12;
        if (c == '_') x = 13;
        if (x) lHash = lHash * 43 + x;
    }
    return lHash;
}

/***********************************************************************
 *
 *           HLPFILE_ReadHlpFile
 *
 *  Read Help file and return pointer to HLPFILE structure.
 *
 *  Input:
 *  LPCSTR lpszPath
 *		Filename of file to open
 *
 *  Output:
 *  LPHLPFILE on Success
 *  NULL if error
 */
LPHLPFILE HLPFILE_ReadHlpFile(LPCSTR lpszPath)
{
    LPHLPFILE hlpfile;

	// If file was already open, then just return pointer to HLPFILE
    for (hlpfile = first_hlpfile; hlpfile; hlpfile = hlpfile->next)
    {
        if (!lstrcmp(lpszPath, hlpfile->lpszPath))
        {
            hlpfile->wRefCount++;
            return hlpfile;
        }
    }

	// Allocate space for header and filename
    hlpfile = (LPHLPFILE)GlobalAllocPtr(GPTR, sizeof(HLPFILE) + lstrlen(lpszPath) + 1);
    if (!hlpfile) return NULL;

    hlpfile->lpszPath           = (char far *)hlpfile + sizeof(HLPFILE);
    hlpfile->lpszTitle          = NULL;
    hlpfile->lpszCopyright      = NULL;
    hlpfile->first_page         = NULL;
    hlpfile->first_macro        = NULL;
    hlpfile->wContextLen        = 0;
    hlpfile->Context            = NULL;
    hlpfile->contents_start     = 0xFFFFFFFF;
    hlpfile->prev               = NULL;
    hlpfile->next               = first_hlpfile;
    hlpfile->wRefCount          = 1;

    hlpfile->numBmps            = 0;
    hlpfile->bmps               = NULL;

    hlpfile->numFonts           = 0;
    hlpfile->fonts              = NULL;

    hlpfile->numWindows         = 0;
    hlpfile->windows            = NULL;

    lstrcpy(hlpfile->lpszPath, lpszPath);

    first_hlpfile = hlpfile;
    if (hlpfile->next) hlpfile->next->prev = hlpfile;

    phrases.offsets = NULL;
    phrases.buffer = NULL;
    topic.map = NULL;
    topic.end = NULL;
    file_buffer = NULL;

    if (!HLPFILE_DoReadHlpFile(hlpfile, lpszPath))
    {
        HLPFILE_FreeHlpFile(hlpfile);
        hlpfile = 0;
    }

    if (phrases.offsets)  GlobalFreePtr(phrases.offsets);
    if (phrases.buffer)   GlobalFreePtr(phrases.buffer);
    if (topic.map)        GlobalFreePtr(topic.map);
    if (file_buffer)      GlobalFreePtr(file_buffer);

    return hlpfile;
}

/***********************************************************************
 *
 *           HLPFILE_DoReadHlpFile
 */
static BOOL HLPFILE_DoReadHlpFile(HLPFILE far *hlpfile, LPCSTR lpszPath)
{
    BOOL        ret;
    HFILE       hFile;
    OFSTRUCT    ofs;
    BYTE far *  buf;
    DWORD       ref = 0x0C;
    unsigned    index, old_index, offset, len, offs;

    hFile = OpenFile(lpszPath, &ofs, OF_READ);
    if (hFile == HFILE_ERROR) return FALSE;

	// Read file to buffer
    ret = HLPFILE_ReadFileToBuffer(hFile);
    _lclose(hFile);
    if (!ret) return FALSE;

    if (!HLPFILE_SystemCommands(hlpfile)) return FALSE;

    /* load phrases support */
    if (!HLPFILE_UncompressLZ77_Phrases(hlpfile))
        HLPFILE_Uncompress_Phrases40(hlpfile);

    if (!HLPFILE_Uncompress_Topic(hlpfile)) return FALSE;
    if (!HLPFILE_ReadFont(hlpfile)) return FALSE;

    buf = topic.map[0];
    old_index = -1;
    offs = 0;
    do
    {
        BYTE far * end;

        /* FIXME this depends on the blocksize, can be 2k in some cases */
        index  = (ref - 0x0C) >> 14;
        offset = (ref - 0x0C) & 0x3fff;

        //WINE_TRACE("ref=%08lx => [%u/%u]\n", ref, index, offset);

        if (index >= topic.wMapLen) {//WINE_WARN("maplen\n")
			; break;
		}
        buf = topic.map[index] + offset;
        if (buf + 0x15 >= topic.end) {//WINE_WARN("extra\n"); 
			break;
		}
        end = min(buf + GET_ULONG(buf, 0), topic.end);
        if (index != old_index) {offs = 0; old_index = index;}

        switch (buf[0x14])
	{
	case 0x02:
            if (!HLPFILE_AddPage(hlpfile, buf, end, index * 0x8000L + offs)) return FALSE;
            break;

	case 0x20:
            if (!HLPFILE_AddParagraph(hlpfile, buf, end, &len)) return FALSE;
            offs += len;
            break;

	case 0x23:
            if (!HLPFILE_AddParagraph(hlpfile, buf, end, &len)) return FALSE;
            offs += len;
            break;

	default:
		{}//WINE_ERR("buf[0x14] = %x\n", buf[0x14]);
	}

        ref = GET_ULONG(buf, 0xc);
    } while (ref != 0xffffffff);

    return HLPFILE_GetContext(hlpfile);
}

/***********************************************************************
 *
 *           HLPFILE_AddPage
 */
static BOOL HLPFILE_AddPage(HLPFILE far *hlpfile, BYTE far *buf, BYTE far * end, unsigned offset)
{
    HLPFILE_PAGE far * page;
    BYTE far *    title;
    UINT          titlesize;
    LPSTR         ptr;
    HLPFILE_MACRO far *macro;

    if (buf + 0x31 > end) {//WINE_WARN("page1\n"); 
		return FALSE;
	};
    title = buf + GET_ULONG(buf, 0x10);
    if (title > end) {//WINE_WARN("page2\n"); 
		return FALSE;
	};

    titlesize = GET_ULONG(buf, 4);
    page = (HLPFILE_PAGE far *) GlobalAllocPtr(GPTR, sizeof(HLPFILE_PAGE) + titlesize + 1);
    if (!page) return FALSE;
    page->lpszTitle = (char*)page + sizeof(HLPFILE_PAGE);

    if (hlpfile->hasPhrases)
    {
        HLPFILE_Uncompress2(title, end, page->lpszTitle, page->lpszTitle + titlesize);
    }
    else
    {
        if (GET_ULONG(buf, 0x4) > GET_ULONG(buf, 0) - GET_ULONG(buf, 0x10))
        {
            /* need to decompress */
            HLPFILE_Uncompress3(page->lpszTitle, page->lpszTitle + titlesize, 
                                title, end);
        }
        else
        {
            _fmemcpy(page->lpszTitle, title, titlesize);
        }
    }

    page->lpszTitle[titlesize] = '\0';

    if (hlpfile->first_page)
    {
        HLPFILE_PAGE far *p;

        for (p = hlpfile->first_page; p->next; p = p->next);
        page->prev = p;
        p->next    = page;
    }
    else
    {
        hlpfile->first_page = page;
        page->prev = NULL;
    }

    page->file            = hlpfile;
    page->next            = NULL;
    page->first_paragraph = NULL;
    page->first_macro     = NULL;
    page->wNumber         = GET_ULONG(buf, 0x21);
    page->offset          = offset;

    page->browse_bwd = GET_ULONG(buf, 0x19);
    page->browse_fwd = GET_ULONG(buf, 0x1D);
/*
    WINE_TRACE("Added page[%d]: title='%s' %08lx << %08x >> %08lx\n",
               page->wNumber, page->lpszTitle, 
               page->browse_bwd, page->offset, page->browse_fwd);
*/
    memset(&attributes, 0, sizeof(attributes));

    /* now load macros */
    ptr = page->lpszTitle + lstrlen(page->lpszTitle) + 1;
    while (ptr < page->lpszTitle + titlesize)
    {
        unsigned len = lstrlen(ptr);
        //WINE_TRACE("macro: %s\n", ptr);
        macro = (HLPFILE_MACRO far *) GlobalAllocPtr(GPTR, sizeof(HLPFILE_MACRO) + len + 1);
        macro->lpszMacro = (char*)(macro + 1);
        _fmemcpy((char*)macro->lpszMacro, ptr, len + 1);
        /* FIXME: shall we really link macro in reverse order ??
         * may produce strange results when played at page opening
         */
        macro->next = page->first_macro;
        page->first_macro = macro;
        ptr += len + 1;
    }

    return TRUE;
}

static long fetch_long(BYTE far ** ptr)
{
    long        ret;

    if (*(*ptr) & 1)
    {
        ret = (*(unsigned long*)(*ptr) - 0x80000000L) / 2;
        (*ptr) += 4;
    }
    else
    {
        ret = (*(unsigned short*)(*ptr) - 0x8000) / 2;
        (*ptr) += 2;
    }

    return ret;
}

static unsigned long fetch_ulong(BYTE far ** ptr)
{
    unsigned long        ret;

    if (*(*ptr) & 1)
    {
        ret = *(unsigned long*)(*ptr) / 2;
        (*ptr) += 4;
    }
    else
    {
        ret = *(unsigned short*)(*ptr) / 2;
        (*ptr) += 2;
    }
    return ret;
}    

static short fetch_short(BYTE far ** ptr)
{
    short       ret;

    if (*(*ptr) & 1)
    {
        ret = (*(unsigned short*)(*ptr) - 0x8000) / 2;
        (*ptr) += 2;
    }
    else
    {
        ret = (*(unsigned char*)(*ptr) - 0x80) / 2;
        (*ptr)++;
    }
    return ret;
}

static unsigned short fetch_ushort(BYTE far ** ptr)
{
    unsigned short ret;

    if (*(*ptr) & 1)
    {
        ret = *(unsigned short*)(*ptr) / 2;
        (*ptr) += 2;
    }
    else
    {
        ret = *(unsigned char*)(*ptr) / 2;
        (*ptr)++;
    }
    return ret;
}

/******************************************************************
 *		HLPFILE_DecompressGfx
 *
 * Decompress the data part of a bitmap or a metafile
 */
static BYTE far *    HLPFILE_DecompressGfx(BYTE far * src, unsigned csz, unsigned sz, BYTE packing)
{
    BYTE far *  dst;
    BYTE far *  tmp;
    BYTE far *  tmp2;
    unsigned    sz77;

    //WINE_TRACE("Unpacking (%d) from %u bytes to %u bytes\n", packing, csz, sz);

    switch (packing)
    {
    case 0: /* uncompressed */
        if (sz != csz)
		  {}//WINE_WARN("Bogus gfx sizes (uncompressed): %u / %u\n", sz, csz);
        dst = src;
        break;
    case 1: /* RunLen */
        tmp = dst = (BYTE far *)GlobalAllocPtr(GPTR, sz);
        if (!dst) return NULL;
        HLPFILE_UncompressRLE(src, src + csz, &tmp, sz);
        if (tmp - dst != sz)
		 {}//WINE_WARN("Bogus gfx sizes (RunLen): %u/%u\n", tmp - dst, sz);
        break;
    case 2: /* LZ77 */
        sz77 = HLPFILE_UncompressedLZ77_Size(src, src + csz);
        dst = GlobalAllocPtr(GPTR, sz77);
        if (!dst) return NULL;
        HLPFILE_UncompressLZ77(src, src + csz, dst);
        if (sz77 != sz)
			{}//WINE_WARN("Bogus gfx sizes (LZ77): %u / %u\n", sz77, sz);
        break;
    case 3: /* LZ77 then RLE */
        sz77 = HLPFILE_UncompressedLZ77_Size(src, src + csz);
        tmp = GlobalAllocPtr(GPTR, sz77);
        if (!tmp) return FALSE;
        HLPFILE_UncompressLZ77(src, src + csz, tmp);
        dst = tmp2 = GlobalAllocPtr(GPTR, sz);
        if (!dst) return FALSE;
        HLPFILE_UncompressRLE(tmp, tmp + sz77, &tmp2, sz);
        if (tmp2 - dst != sz)
			{}//WINE_WARN("Bogus gfx sizes (LZ77+RunLen): %u / %u\n", tmp2 - dst, sz);
        GlobalFreePtr(tmp);
        break;
    default:
		{}//WINE_FIXME("Unsupported packing %u\n", packing);
        return NULL;
    }
    return dst;
}

/******************************************************************
 *		HLPFILE_LoadBitmap
 *
 *
 */
static BOOL HLPFILE_LoadBitmap(BYTE far * beg, BYTE type, BYTE pack, 
                               HLPFILE_PARAGRAPH far * paragraph)
{
    BYTE far *          ptr;
    BYTE far *          pict_beg;
    BITMAPINFO far *    bi;
    unsigned long       off, csz;
    HDC                 hdc;

    bi = (BITMAPINFO far *) GlobalAllocPtr(GPTR, sizeof(*bi));
    if (!bi) return FALSE;

    ptr = beg + 2; /* for type and pack */

    bi->bmiHeader.biSize          = sizeof(bi->bmiHeader);
    bi->bmiHeader.biXPelsPerMeter = fetch_ulong(&ptr);
    bi->bmiHeader.biYPelsPerMeter = fetch_ulong(&ptr);
    bi->bmiHeader.biPlanes        = fetch_ushort(&ptr);
    bi->bmiHeader.biBitCount      = fetch_ushort(&ptr);
    bi->bmiHeader.biWidth         = fetch_ulong(&ptr);
    bi->bmiHeader.biHeight        = fetch_ulong(&ptr);
    bi->bmiHeader.biClrUsed       = fetch_ulong(&ptr);
    bi->bmiHeader.biClrImportant  = fetch_ulong(&ptr);
    bi->bmiHeader.biCompression   = BI_RGB;
    if (bi->bmiHeader.biBitCount > 32) {}//WINE_FIXME("Unknown bit count %u\n", bi->bmiHeader.biBitCount);
    if (bi->bmiHeader.biPlanes != 1) {}//WINE_FIXME("Unsupported planes %u\n", bi->bmiHeader.biPlanes);
    bi->bmiHeader.biSizeImage = (((bi->bmiHeader.biWidth * bi->bmiHeader.biBitCount + 31) & ~31) / 8) * bi->bmiHeader.biHeight;
    /*WINE_TRACE("planes=%d bc=%d size=(%ld,%ld)\n",
               bi->bmiHeader.biPlanes, bi->bmiHeader.biBitCount, 
               bi->bmiHeader.biWidth, bi->bmiHeader.biHeight);*/

    csz = fetch_ulong(&ptr);
    fetch_ulong(&ptr); /* hotspot size */

    off = GET_ULONG(ptr, 0);     ptr += 4;
    /* GET_UINT(ptr, 0); hotspot offset */ ptr += 4;
    
    /* now read palette info */
    if (type == 0x06)
    {
        unsigned nc = bi->bmiHeader.biClrUsed;
        unsigned i;
        
        /* not quite right, especially for bitfields type of compression */
        if (!nc && bi->bmiHeader.biBitCount <= 8)
            nc = 1 << bi->bmiHeader.biBitCount;
        
        bi = (BITMAPINFO far *)GlobalLock(GlobalReAlloc(GlobalPtrHandle(bi), sizeof(*bi) + nc * sizeof(RGBQUAD), 0));
        if (!bi) return FALSE;
        for (i = 0; i < nc; i++)
        {
            bi->bmiColors[i].rgbBlue     = ptr[0];
            bi->bmiColors[i].rgbGreen    = ptr[1];
            bi->bmiColors[i].rgbRed      = ptr[2];
            bi->bmiColors[i].rgbReserved = 0;
            ptr += 4;
        }
    }
    pict_beg = HLPFILE_DecompressGfx(beg + off, csz, bi->bmiHeader.biSizeImage, pack);
    
    paragraph->u.gfx.u.bmp.hBitmap = CreateDIBitmap(hdc = GetDC(0), &bi->bmiHeader, 
                                                    CBM_INIT, pict_beg, 
                                                    bi, DIB_RGB_COLORS);
    ReleaseDC(0, hdc);      
    if (!paragraph->u.gfx.u.bmp.hBitmap)
		{}//WINE_ERR("Couldn't create bitmap\n");
    
    GlobalFreePtr(bi);
    if (pict_beg != beg + off) GlobalFreePtr(pict_beg);

    return TRUE;
}

/******************************************************************
 *		HLPFILE_LoadMetaFile
 *
 *
 */
static BOOL     HLPFILE_LoadMetaFile(BYTE far * beg, BYTE pack, HLPFILE_PARAGRAPH far * paragraph)
{
    BYTE far *          ptr;
    unsigned long       size, csize;
    unsigned long       off, hsoff;
    BYTE far *          bits;
    METAFILEPICT        mfp;

    //WINE_TRACE("Loading metafile\n");

    ptr = beg + 2; /* for type and pack */

    mfp.mm = fetch_ushort(&ptr); /* mapping mode */

    mfp.xExt = GET_USHORT(ptr, 0);
    mfp.yExt = GET_USHORT(ptr, 2);
    ptr += 4;

    size = fetch_ulong(&ptr); /* decompressed size */
    csize = fetch_ulong(&ptr); /* compressed size */
    fetch_ulong(&ptr); /* hotspot size */
    off = GET_ULONG(ptr, 0);
    hsoff = GET_ULONG(ptr, 4);
    ptr += 8;

    /*WINE_TRACE("sz=%lu csz=%lu (%ld,%ld) offs=%lu/%u,%lu\n", 
               size, csize, mfp.xExt, mfp.yExt, off, ptr - beg, hsoff);
*/
    bits = HLPFILE_DecompressGfx(beg + off, csize, size, pack);
    if (!bits) return FALSE;

    paragraph->cookie = para_metafile;

    mfp.hMF = 0;

    paragraph->u.gfx.u.mf.hMetaFile = SetMetaFileBits(/*size,*/ GlobalPtrHandle(bits));

    if (!paragraph->u.gfx.u.mf.hMetaFile)
		{}//WINE_FIXME("Couldn't load metafile\n");

    if (bits != beg + off) GlobalFreePtr(bits);

    paragraph->u.gfx.u.mf.mfSize.cx = mfp.xExt;
    paragraph->u.gfx.u.mf.mfSize.cy = mfp.yExt;

    return TRUE;
}

/******************************************************************
 *		HLPFILE_LoadGfxByAddr
 *
 *
 */
static  BOOL    HLPFILE_LoadGfxByAddr(HLPFILE far * hlpfile, BYTE far * ref,
                                      unsigned long size, 
                                      HLPFILE_PARAGRAPH far * paragraph)
{
    unsigned    i, numpict;

    numpict = GET_USHORT(ref, 2);
    //WINE_TRACE("Got picture magic=%04x #=%d\n", 
      //         GET_USHORT(ref, 0), numpict);

    for (i = 0; i < numpict; i++)
    {
        BYTE far * beg;
        BYTE far * ptr;
        BYTE    type, pack;

        //WINE_TRACE("Offset[%d] = %x\n", i, GET_UINT(ref, (1 + i) * 4));
        beg = ptr = ref + GET_ULONG(ref, (1 + i) * 4);

        type = *ptr++;
        pack = *ptr++;
        
        switch (type)
        {
        case 5: /* device dependent bmp */
        case 6: /* device independent bmp */
            HLPFILE_LoadBitmap(beg, type, pack, paragraph);
            break;
        case 8: 
            HLPFILE_LoadMetaFile(beg, pack, paragraph);
            break;
        default: {//WINE_FIXME("Unknown type %u\n", type); 
			return FALSE;
		}
        }

        /* FIXME: hotspots */

        /* FIXME: implement support for multiple picture format */
        if (numpict != 1) {}//WINE_FIXME("Supporting only one bitmap format per logical bitmap (for now). Using first format\n");
        break;
    }
    return TRUE;
}

/******************************************************************
 *		HLPFILE_LoadGfxByIndex
 *
 *
 */
static  BOOL    HLPFILE_LoadGfxByIndex(HLPFILE far *hlpfile, unsigned index, 
                                       HLPFILE_PARAGRAPH far * paragraph)
{
    char        tmp[16];
    BYTE    far *ref;
	BYTE    far *end;
    BOOL        ret;

    //WINE_TRACE("Loading picture #%d\n", index);

    if (index < hlpfile->numBmps && hlpfile->bmps[index] != 0)
    {
        paragraph->u.gfx.u.bmp.hBitmap = hlpfile->bmps[index];
        return TRUE;
    }

    sprintf(tmp, "|bm%u", index);

    if (!HLPFILE_FindSubFile(tmp, &ref, &end)) {//WINE_WARN("no sub file\n"); 
		return FALSE;
	}

    ref += 9;

    ret = HLPFILE_LoadGfxByAddr(hlpfile, ref, end - ref, paragraph);

    /* cache bitmap */
    if (ret && paragraph->cookie == para_bitmap)
    {
        if (index >= hlpfile->numBmps)
        {
            hlpfile->numBmps = index + 1;
	    if (hlpfile->bmps)
        	hlpfile->bmps = (HBITMAP far *)GlobalLock(GlobalReAlloc(GlobalPtrHandle(hlpfile->bmps), 
                                        hlpfile->numBmps * sizeof(hlpfile->bmps[0]), 0));
	    else
	    	hlpfile->bmps = (HBITMAP far *)GlobalAllocPtr(GPTR, 
                                        hlpfile->numBmps * sizeof(hlpfile->bmps[0]));

        }
        hlpfile->bmps[index] = paragraph->u.gfx.u.bmp.hBitmap;
    }
    return ret;
}

/******************************************************************
 *		HLPFILE_AllocLink
 *
 *
 */
static HLPFILE_LINK far * HLPFILE_AllocLink(int cookie, const char far * str, LONG hash,
                                             BOOL clrChange, unsigned wnd)
{
    HLPFILE_LINK far *  link;

    /* FIXME: should build a string table for the attributes.link.lpszPath
     * they are reallocated for each link
     */
    link = (HLPFILE_LINK far *) GlobalAllocPtr(GPTR, sizeof(HLPFILE_LINK) + lstrlen(str) + 1);
    if (!link) return NULL;

    link->cookie     = cookie;
    link->lpszString = (char*)link + sizeof(HLPFILE_LINK);
    lstrcpy((char*)link->lpszString, str);
    link->lHash      = hash;
    link->bClrChange = clrChange ? 1 : 0;
    link->window     = wnd;
    link->wRefCount   = 1;

    /*WINE_TRACE("Link[%d] to %s@%08lx:%d\n",
               link->cookie, link->lpszString, 
               link->lHash, link->window);*/
    return link;
}

/***********************************************************************
 *
 *           HLPFILE_AddParagraph
 */
static BOOL HLPFILE_AddParagraph(HLPFILE far *hlpfile, BYTE far *  buf, BYTE far *end, unsigned* len)
{
    HLPFILE_PAGE  far *page;
    HLPFILE_PARAGRAPH far *paragraph;
	HLPFILE_PARAGRAPH far * far *paragraphptr;
    UINT               textsize;
    BYTE far          *format;
	BYTE far          *format_end;
	BYTE far          *text;
	BYTE far          *text_end;
    long               size;
    unsigned short     bits;
    unsigned           nc, ncol = 1;

    if (!hlpfile->first_page) {//WINE_WARN("no page\n"); 
		return FALSE;
	};

    for (page = hlpfile->first_page; page->next; page = page->next) /* Nothing */;
    for (paragraphptr = &page->first_paragraph; *paragraphptr;
         paragraphptr = &(*paragraphptr)->next) /* Nothing */;

    if (buf + 0x19 > end) {//WINE_WARN("header too small\n"); 
		return FALSE;
	};

    size = GET_ULONG(buf, 0x4);
    text = GlobalAllocPtr(GPTR, size);
    if (!text) return FALSE;
    if (hlpfile->hasPhrases)
    {
        HLPFILE_Uncompress2(buf + GET_ULONG(buf, 0x10), end, text, text + size);
    }
    else
    {
        if (GET_ULONG(buf, 0x4) > GET_ULONG(buf, 0) - GET_ULONG(buf, 0x10))
        {
            /* block is compressed */
            HLPFILE_Uncompress3(text, text + size, buf + GET_ULONG(buf, 0x10), end);
        }
        else
        {
            text = buf + GET_ULONG(buf, 0x10);
        }
    }
    text_end = text + size;

    format = buf + 0x15;
    format_end = buf + GET_ULONG(buf, 0x10);

    fetch_long(&format);
    *len = fetch_ushort(&format);

    if (buf[0x14] == 0x23)
    {
        char    type;

        ncol = *format++;

        //WINE_TRACE("#cols %u\n", ncol);
        type = *format++;
        if (type == 0 || type == 2)
            format += 2;
        format += ncol * 4;
    }

    for (nc = 0; nc < ncol; nc++)
    {
        //WINE_TRACE("looking for format at offset %u for column %d\n", format - (buf + 0x15), nc);
        if (buf[0x14] == 0x23)
            format += 5;
        format += 4;
        bits = GET_USHORT(format, 0); format += 2;
        if (bits & 0x0001) fetch_long(&format);
        if (bits & 0x0002) fetch_short(&format);
        if (bits & 0x0004) fetch_short(&format);
        if (bits & 0x0008) fetch_short(&format);
        if (bits & 0x0010) fetch_short(&format);
        if (bits & 0x0020) fetch_short(&format);
        if (bits & 0x0040) fetch_short(&format);
        if (bits & 0x0100) format += 3;
        if (bits & 0x0200)
        {
            int                 ntab = fetch_short(&format);
            unsigned short      ts;

            while (ntab-- > 0)
            {
                ts = fetch_ushort(&format);
                if (ts & 0x4000) fetch_ushort(&format);
            }
        }
        /* 0x0400, 0x0800 and 0x1000 don't need space */
        if ((bits & 0xE080) != 0) 
			{}//WINE_FIXME("Unsupported bits %04x, potential trouble ahead\n", bits);

        while (text < text_end && format < format_end)
        {
//            WINE_TRACE("Got text: '%s' (%p/%p - %p/%p)\n", text, text, text_end, format, format_end);
            textsize = lstrlen(text) + 1;
            if (textsize > 1)
            {
                paragraph = (HLPFILE_PARAGRAPH far *) GlobalAllocPtr(GPTR,
                                      sizeof(HLPFILE_PARAGRAPH) + textsize);
                if (!paragraph) return FALSE;
                *paragraphptr = paragraph;
                paragraphptr = &paragraph->next;

                paragraph->next            = NULL;
                paragraph->link            = attributes.link;
                if (paragraph->link) paragraph->link->wRefCount++;
                paragraph->cookie          = para_normal_text;
                paragraph->u.text.wFont    = attributes.wFont;
                paragraph->u.text.wVSpace  = attributes.wVSpace;
                paragraph->u.text.wHSpace  = attributes.wHSpace;
                paragraph->u.text.wIndent  = attributes.wIndent;
                paragraph->u.text.lpszText = (char*)paragraph + sizeof(HLPFILE_PARAGRAPH);
                lstrcpy(paragraph->u.text.lpszText, text);

                attributes.wVSpace = 0;
                attributes.wHSpace = 0;
            }
            /* else: null text, keep on storing attributes */
            text += textsize;

	    if (*format == 0xff)
            {
                format++;
                break;
            }

            //WINE_TRACE("format=%02x\n", *format);
            switch (*format)
            {
            case 0x20:
                //WINE_FIXME("NIY20\n");
                format += 5;
                break;

            case 0x21:
                //WINE_FIXME("NIY21\n");
                format += 3;
                break;

	    case 0x80:
                attributes.wFont = GET_USHORT(format, 1);
                //WINE_TRACE("Changing font to %d\n", attributes.wFont);
                format += 3;
                break;

	    case 0x81:
                attributes.wVSpace++;
                format += 1;
                break;

	    case 0x82:
                attributes.wVSpace++;
                attributes.wIndent = 0;
                format += 1;
                break;

	    case 0x83:
                attributes.wIndent++;
                format += 1;
                break;

#if 0
	    case 0x84:
                format += 3;
                break;
#endif

	    case 0x86:
	    case 0x87:
	    case 0x88:
                {
                    BYTE    pos = (*format - 0x86);
                    BYTE    type = format[1];
                    long    size;

                    format += 2;
                    size = fetch_long(&format);

                    paragraph = (HLPFILE_PARAGRAPH far *)GlobalAllocPtr(GPTR,
                                          sizeof(HLPFILE_PARAGRAPH) + textsize);
                    if (!paragraph) return FALSE;
                    *paragraphptr = paragraph;
                    paragraphptr = &paragraph->next;

                    paragraph->next        = NULL;
                    paragraph->link        = attributes.link;
                    if (paragraph->link) paragraph->link->wRefCount++;
                    paragraph->cookie      = para_bitmap;
                    paragraph->u.gfx.pos   = pos;
                    switch (type)
                    {
                    case 0x22:
                        fetch_ushort(&format); /* hot spot */
                        /* fall thru */
                    case 0x03:
                        switch (GET_SHORT(format, 0))
                        {
                        case 0:
                            HLPFILE_LoadGfxByIndex(hlpfile, GET_SHORT(format, 2), 
                                                   paragraph);
                            break;
                        case 1:
                            //WINE_FIXME("does it work ??? %x<%lu>#%u\n", 
                              //         GET_SHORT(format, 0), 
                                //       size, GET_SHORT(format, 2));
                            HLPFILE_LoadGfxByAddr(hlpfile, format + 2, size - 4, 
                                                  paragraph);
                            break;
                        default:
                            //WINE_FIXME("??? %u\n", GET_SHORT(format, 0));
                            break;
                        }
                        break;
                    case 0x05:
                        //WINE_FIXME("Got an embedded element %s\n", format + 6);
                        break;
                    default:
                        //WINE_FIXME("Got a type %d picture\n", type);
                        break;
                    }
                    if (attributes.wVSpace) paragraph->u.gfx.pos |= 0x8000;

                    format += size;
                }
                break;

	    case 0x89:
                HLPFILE_FreeLink(attributes.link);
                attributes.link = NULL;
                format += 1;
                break;

            case 0x8B:
            case 0x8C:
                //WINE_FIXME("NIY non-break space/hyphen\n");
                format += 1;
                break;

#if 0
	    case 0xA9:
                format += 2;
                break;
#endif

            case 0xC8:
            case 0xCC:
                //WINE_TRACE("macro => %s\n", format + 3);
                HLPFILE_FreeLink(attributes.link);
                attributes.link = HLPFILE_AllocLink(hlp_link_macro, format + 3, 
                                                    0, !(*format & 4), -1);
                format += 3 + GET_USHORT(format, 1);
                break;

            case 0xE0:
            case 0xE1:
                //WINE_WARN("jump topic 1 => %u\n", GET_UINT(format, 1));
                format += 5;
                break;

	    case 0xE2:
	    case 0xE3:
            case 0xE6:
            case 0xE7:
                HLPFILE_FreeLink(attributes.link);
                attributes.link = HLPFILE_AllocLink((*format & 1) ? hlp_link_link : hlp_link_popup,
                                                    hlpfile->lpszPath, 
                                                    GET_ULONG(format, 1), 
                                                    !(*format & 4), -1);
                format += 5;
                break;

	    case 0xEA:
            case 0xEB:
            case 0xEE:
            case 0xEF:
                {
                    LPSTR       ptr = format + 8;
                    BYTE        type = format[3];
                    int         wnd = -1;
                    LPSTR       str;

                    if (type == 1) wnd = *ptr++;
                    if (type == 4 || type == 6)
                    {
                        str = ptr;
                        ptr += lstrlen(ptr) + 1;
                    }
                    else
                        str = hlpfile->lpszPath;
                    if (type == 6)
                    {
                        for (wnd = hlpfile->numWindows - 1; wnd >= 0; wnd--)
                        {
                            if (!lstrcmp(ptr, hlpfile->windows[wnd].name)) break;
                        }
                        if (wnd == -1)
							{}//WINE_WARN("Couldn't find window info for %s\n", ptr);
                    }
                    HLPFILE_FreeLink(attributes.link);
                    attributes.link = HLPFILE_AllocLink((*format & 4) ? hlp_link_link : hlp_link_popup,
                                                        str, GET_ULONG(format, 4),
                                                        !(*format & 1), wnd);
                }
                format += 3 + GET_USHORT(format, 1);
                break;

	    default:
                //WINE_WARN("format %02x\n", *format);
                format++;
	    }
	}
    }
    if (text_end != buf + GET_ULONG(buf, 0x10) + size)
        GlobalFreePtr(text_end - size);
    return TRUE;
}

/******************************************************************
 *		HLPFILE_ReadFont
 *
 *
 */
static BOOL HLPFILE_ReadFont(HLPFILE far * hlpfile)
{
    BYTE   far  *ref;
	BYTE   far  *end;
    unsigned    i, len, idx;
    unsigned    face_num, dscr_num, face_offset, dscr_offset;
    BYTE        flag, family;

    if (!HLPFILE_FindSubFile("|FONT", &ref, &end))
    {
        //WINE_WARN("no subfile FONT\n");
        hlpfile->numFonts = 0;
        hlpfile->fonts = NULL;
        return FALSE;
    }

    ref += 9;

    face_num    = GET_USHORT(ref, 0);
    dscr_num    = GET_USHORT(ref, 2);
    face_offset = GET_USHORT(ref, 4);
    dscr_offset = GET_USHORT(ref, 6);

    //WINE_TRACE("Got NumFacenames=%u@%u NumDesc=%u@%u\n",
      //         face_num, face_offset, dscr_num, dscr_offset);

    hlpfile->numFonts = dscr_num;
    hlpfile->fonts = (HLPFILE_FONT far *)GlobalAllocPtr(GPTR, sizeof(HLPFILE_FONT) * dscr_num);

    len = (dscr_offset - face_offset) / face_num;
/* EPP     for (i = face_offset; i < dscr_offset; i += len) */
/* EPP         WINE_FIXME("[%d]: %*s\n", i / len, len, ref + i); */
    for (i = 0; i < dscr_num; i++)
    {
        flag = ref[dscr_offset + i * 11 + 0];
        family = ref[dscr_offset + i * 11 + 2];

        hlpfile->fonts[i].LogFont.lfHeight = -ref[dscr_offset + i * 11 + 1] / 2;
        hlpfile->fonts[i].LogFont.lfWidth = 0;
        hlpfile->fonts[i].LogFont.lfEscapement = 0;
        hlpfile->fonts[i].LogFont.lfOrientation = 0;
        hlpfile->fonts[i].LogFont.lfWeight = (flag & 1) ? 700 : 400;
        hlpfile->fonts[i].LogFont.lfItalic = (flag & 2) ? TRUE : FALSE;
        hlpfile->fonts[i].LogFont.lfUnderline = (flag & 4) ? TRUE : FALSE;
        hlpfile->fonts[i].LogFont.lfStrikeOut = (flag & 8) ? TRUE : FALSE;
        hlpfile->fonts[i].LogFont.lfCharSet = ANSI_CHARSET;
        hlpfile->fonts[i].LogFont.lfOutPrecision = OUT_DEFAULT_PRECIS;
        hlpfile->fonts[i].LogFont.lfClipPrecision = CLIP_DEFAULT_PRECIS;
        hlpfile->fonts[i].LogFont.lfQuality = DEFAULT_QUALITY;
        hlpfile->fonts[i].LogFont.lfPitchAndFamily = DEFAULT_PITCH;

        switch (family)
        {
        case 0x01: hlpfile->fonts[i].LogFont.lfPitchAndFamily |= FF_MODERN;     break;
        case 0x02: hlpfile->fonts[i].LogFont.lfPitchAndFamily |= FF_ROMAN;      break;
        case 0x03: hlpfile->fonts[i].LogFont.lfPitchAndFamily |= FF_SWISS;      break;
        case 0x04: hlpfile->fonts[i].LogFont.lfPitchAndFamily |= FF_SCRIPT;     break;
        case 0x05: hlpfile->fonts[i].LogFont.lfPitchAndFamily |= FF_DECORATIVE; break;
        default: {}//WINE_FIXME("Unknown family %u\n", family);
        }
        idx = GET_USHORT(ref, dscr_offset + i * 11 + 3);

        if (idx < face_num)
        {
            lstrcpyn(hlpfile->fonts[i].LogFont.lfFaceName, ref + face_offset + idx * len, min(len, LF_FACESIZE - 1));
            hlpfile->fonts[i].LogFont.lfFaceName[min(len, LF_FACESIZE - 1) + 1] = '\0';
        }
        else
        {
            //WINE_FIXME("Too high face ref (%u/%u)\n", idx, face_num);
            strcpy(hlpfile->fonts[i].LogFont.lfFaceName, "Helv");
        }
        hlpfile->fonts[i].hFont = 0;
        hlpfile->fonts[i].color = RGB(ref[dscr_offset + i * 11 + 5],
                                      ref[dscr_offset + i * 11 + 6],
                                      ref[dscr_offset + i * 11 + 7]);
#define X(b,s) ((flag & (1 << b)) ? "-"s: "")
        //WINE_TRACE("Font[%d]: flags=%02x%s%s%s%s%s%s pSize=%u family=%u face=%s[%u] color=%08x\n",
          //         i, flag,
            //       X(0, "bold"),
              //     X(1, "italic"),
                //   X(2, "underline"),
                  // X(3, "strikeOut"),
                   //X(4, "dblUnderline"),
                   //X(5, "smallCaps"),
                   //ref[dscr_offset + i * 11 + 1],
                   //family,
                   //hlpfile->fonts[i].LogFont.lfFaceName, idx,
                   //GET_UINT(ref, dscr_offset + i * 11 + 5) & 0x00FFFFFF);
    }
    return TRUE;
}

/***********************************************************************
 *
 *           HLPFILE_ReadFileToBuffer
 *
 *  Read help file to buffer
 *
 * Returns:
 *	TRUE Success
 *	FALSE Error
 */
static BOOL HLPFILE_ReadFileToBuffer(HFILE hFile)
{
    BYTE   dummy[1];
    DWORD  size;
	HELPHEADER header;

    if (_hread(hFile, &header, sizeof(HELPHEADER)) != sizeof(HELPHEADER))
	{
#ifdef DEBUG
		OutputDebugString("HLPFILE_ReadFileToBuffer: Header read error\n");
#endif
		return FALSE;
	};

    /* sanity checks */
    if (header.Magic != 0x00035F3F) 
    {
#ifdef DEBUG
		OutputDebugString("HLPFILE_ReadFileToBuffer: wrong header\n");
#endif
		return FALSE;
	};

    size = header.EntireFileSize;
    file_buffer = GlobalAllocPtr(GPTR, size + 1); // Allocate memory for file_buffer plus protective zero byte
    if (!file_buffer) return FALSE;

    _fmemcpy(file_buffer, &header, sizeof(HELPHEADER)); // Copy header to buffer
	// read rest of file
    if (_hread(hFile, file_buffer + sizeof(HELPHEADER), size - sizeof(HELPHEADER)) != size - sizeof(HELPHEADER))
    {
#ifdef DEBUG
		OutputDebugString("HLPFILE_ReadFileToBuffer: Error read file to buffer\n");
#endif
		return FALSE;
	};

	// Try to read after file end
    if (_hread(hFile, dummy, sizeof(dummy)) != 0) 
	{
#ifdef DEBUG
		OutputDebugString("HLPFILE_ReadFileToBuffer: Warning: Extra bytes after file end\n");
#endif
	};

    file_buffer[size] = '\0'; /* FIXME: was '0', sounds ackward to me */

    return TRUE;
}

/***********************************************************************
 *
 *           HLPFILE_FindSubFile
 *
 *  Find subfile with name pointed by lpszName and return start and end pointers in buffer
 *
 *  Input:
 *	LPCSTR lpszName
 *  Output:
 *  subbuf - start address
 *  subend - end address
 *  FALSE - Failure
 *  TRUE - Success
 */
static BOOL HLPFILE_FindSubFile(LPCSTR lpszName, BYTE far * far*subbuf, BYTE far * far *subend)
{
    BYTE far *root = file_buffer + ((LPHELPHEADER)file_buffer)->DirectoryStart;//GET_ULONG(file_buffer,  4);
    BYTE far *end  = file_buffer + ((LPHELPHEADER)file_buffer)->EntireFileSize;//GET_ULONG(file_buffer, 12);
    BYTE far *ptr;
    LPBTREEHEADER bth;

    WORD    pgsize;
    unsigned    pglast;
    unsigned    nentries;
    unsigned    i, n;

    bth = (LPBTREEHEADER)(root + sizeof(FILEHEADER));//9;

    /* FIXME: this should be using the EnumBTree functions from this file */
    pgsize = bth->PageSize;//GET_USHORT(bth, 4);
    //WINE_TRACE("%s => pgsize=%u #pg=%u rootpg=%u #lvl=%u\n", 
      //         name, pgsize, GET_USHORT(bth, 30), GET_USHORT(bth, 26), GET_USHORT(bth, 32));

    //ptr = bth + 38 + GET_USHORT(bth, 26) * pgsize;
	ptr = (BYTE far *)(bth + sizeof(BTREEHEADER) + bth->RootPage * pgsize);

//    for (n = 1; n < GET_USHORT(bth, 32); n++)
    for (n = 1; n < bth->NLevels; n++)
    {
        nentries = GET_USHORT(ptr, 2);
        pglast = GET_USHORT(ptr, 4);
        //WINE_TRACE("[%u]: #entries=%u next=%u\n", n, nentries, pglast);

        ptr += 6;
        for (i = 0; i < nentries; i++)
        {
            //WINE_TRACE("<= %s\n", ptr);
            if (lstrcmp(lpszName, ptr) < 0) break;
            ptr += lstrlen(ptr) + 1;
            pglast = GET_USHORT(ptr, 0);
            ptr += 2;
        }
//        ptr = bth + 38 + pglast * pgsize;
	ptr = (BYTE far *)(bth + sizeof(BTREEHEADER) + pglast * pgsize);
    }

    nentries = GET_USHORT(ptr, 2);
    ptr += 8;
    for (i = 0; i < nentries; i++)
    {
        LPSTR   fname = ptr;
        ptr += lstrlen(fname) + 1;
        //WINE_TRACE("\\- %s\n", fname);
        if (lstrcmp(fname, lpszName) == 0)
        {
            *subbuf = file_buffer + GET_ULONG(ptr, 0);
            *subend = *subbuf + GET_ULONG(*subbuf, 0);
            if (file_buffer > *subbuf || *subbuf > *subend || *subend > end)
	    {
              //  WINE_WARN("size mismatch\n");
                return FALSE;
	    }
            return TRUE;
        }
        ptr += 4;
    }

    return FALSE;
}

/***********************************************************************
 *
 *           HLPFILE_SystemCommands
 */
static BOOL HLPFILE_SystemCommands(HLPFILE far * hlpfile)
{
    BYTE far *buf;
	BYTE far *ptr;
	BYTE far *end;
    HLPFILE_MACRO far *macro;
	HLPFILE_MACRO far * far *m;
    LPSTR p;
    unsigned short magic, minor, major, flags;

    hlpfile->lpszTitle = NULL;

    if (!HLPFILE_FindSubFile("|SYSTEM", &buf, &end)) return FALSE;

    magic = GET_USHORT(buf + 9, 0);
    minor = GET_USHORT(buf + 9, 2);
    major = GET_USHORT(buf + 9, 4);
    /* gen date on 4 bytes */
    flags = GET_USHORT(buf + 9, 10);
    //WINE_TRACE("Got system header: magic=%04x version=%d.%d flags=%04x\n",
               //magic, major, minor, flags);
    if (magic != 0x036C || major != 1)
    {//{WINE_WARN("Wrong system header\n"); 
		return FALSE;
	}
    if (minor <= 16) {//WINE_WARN("too old file format (NIY)\n"); 
		return FALSE;
	}
    if (flags & 8) {//WINE_WARN("Unsupported yet page size\n"); 
		return FALSE;
	}

    hlpfile->version = minor;
    hlpfile->flags = flags;

    for (ptr = buf + 0x15; ptr + 4 <= end; ptr += GET_USHORT(ptr, 2) + 4)
    {
        switch (GET_USHORT(ptr, 0))
	{
	case 1:
            if (hlpfile->lpszTitle) {//WINE_WARN("title\n"); 
				break;
			}
            hlpfile->lpszTitle = GlobalAllocPtr(GPTR, lstrlen(ptr + 4) + 1);
            if (!hlpfile->lpszTitle) return FALSE;
            lstrcpy(hlpfile->lpszTitle, ptr + 4);
            //WINE_TRACE("Title: %s\n", hlpfile->lpszTitle);
            break;

	case 2:
            if (hlpfile->lpszCopyright) {//WINE_WARN("copyright\n"); 
				break;
			}
            hlpfile->lpszCopyright = GlobalAllocPtr(GPTR, lstrlen(ptr + 4) + 1);
            if (!hlpfile->lpszCopyright) return FALSE;
            lstrcpy(hlpfile->lpszCopyright, ptr + 4);
            //WINE_TRACE("Copyright: %s\n", hlpfile->lpszCopyright);
            break;

	case 3:
            if (GET_USHORT(ptr, 2) != 4) {//WINE_WARN("system3\n");
				break;
			}
            hlpfile->contents_start = GET_ULONG(ptr, 4);
            //WINE_TRACE("Setting contents start at %08lx\n", hlpfile->contents_start);
            break;

	case 4:
            macro = (HLPFILE_MACRO far *)GlobalAllocPtr(GPTR, sizeof(HLPFILE_MACRO) + lstrlen(ptr + 4) + 1);
            if (!macro) break;
            p = (char far *)macro + sizeof(HLPFILE_MACRO);
            lstrcpy(p, (LPSTR)ptr + 4);
            macro->lpszMacro = p;
            macro->next = 0;
            for (m = &hlpfile->first_macro; *m; m = &(*m)->next);
            *m = macro;
            break;

        case 6:
            if (GET_USHORT(ptr, 2) != 90) {//WINE_WARN("system6\n");
				break;
			}

	    if (hlpfile->windows) 
        	hlpfile->windows = (HLPFILE_WINDOWINFO far *)GlobalLock(GlobalReAlloc(GlobalPtrHandle(hlpfile->windows), 
                                           sizeof(HLPFILE_WINDOWINFO) * ++hlpfile->numWindows, 0));
	    else 
        	hlpfile->windows = (HLPFILE_WINDOWINFO far *)GlobalAllocPtr(GPTR, 
                                           sizeof(HLPFILE_WINDOWINFO) * ++hlpfile->numWindows);
	    
            if (hlpfile->windows)
            {
                unsigned flags = GET_USHORT(ptr, 4);
                HLPFILE_WINDOWINFO* wi = &hlpfile->windows[hlpfile->numWindows - 1];

                if (flags & 0x0001) lstrcpy(wi->type, ptr + 6); else wi->type[0] = '\0';
                if (flags & 0x0002) lstrcpy(wi->name, ptr + 16); else wi->name[0] = '\0';
                if (flags & 0x0004) lstrcpy(wi->caption, ptr + 25); else lstrcpyn(wi->caption, hlpfile->lpszTitle, sizeof(wi->caption));
                wi->origin.x = (flags & 0x0008) ? GET_USHORT(ptr, 76) : CW_USEDEFAULT;
                wi->origin.y = (flags & 0x0010) ? GET_USHORT(ptr, 78) : CW_USEDEFAULT;
                wi->size.cx = (flags & 0x0020) ? GET_USHORT(ptr, 80) : CW_USEDEFAULT;
                wi->size.cy = (flags & 0x0040) ? GET_USHORT(ptr, 82) : CW_USEDEFAULT;
                wi->style = (flags & 0x0080) ? GET_USHORT(ptr, 84) : SW_SHOW;
                wi->sr_color = (flags & 0x0100) ? GET_ULONG(ptr, 86) : 0xFFFFFF;
                wi->nsr_color = (flags & 0x0200) ? GET_ULONG(ptr, 90) : 0xFFFFFF;
                //WINE_TRACE("System-Window: flags=%c%c%c%c%c%c%c%c type=%s name=%s caption=%s (%ld,%ld)x(%ld,%ld)\n",
                //           flags & 0x0001 ? 'T' : 't',
                //           flags & 0x0002 ? 'N' : 'n',
                           //flags & 0x0004 ? 'C' : 'c',
                           //flags & 0x0008 ? 'X' : 'x',
                           //flags & 0x0010 ? 'Y' : 'y',
                           //flags & 0x0020 ? 'W' : 'w',
                           //flags & 0x0040 ? 'H' : 'h',
                           //flags & 0x0080 ? 'S' : 's',
                           //wi->type, wi->name, wi->caption, wi->origin.x, wi->origin.y,
                           //wi->size.cx, wi->size.cy);
            }
            break;
	default:
		{}//WINE_WARN("Unsupported SystemRecord[%d]\n", GET_USHORT(ptr, 0));
	}
    }
    return TRUE;
}

/***********************************************************************
 *
 *           HLPFILE_UncompressedLZ77_Size
 */
static int HLPFILE_UncompressedLZ77_Size(BYTE far * ptr, BYTE far * end)
{
    int  i, newsize = 0;

    while (ptr < end)
    {
        int mask = *ptr++;
        for (i = 0; i < 8 && ptr < end; i++, mask >>= 1)
	{
            if (mask & 1)
	    {
                int code = GET_USHORT(ptr, 0);
                int len  = 3 + (code >> 12);
                newsize += len;
                ptr     += 2;
	    }
            else newsize++, ptr++;
	}
    }

    return newsize;
}

/***********************************************************************
 *
 *           HLPFILE_UncompressLZ77
 */
static BYTE far * HLPFILE_UncompressLZ77(BYTE far * ptr, BYTE far * end, BYTE far *newptr)
{
    int i;

#ifdef DEBUG
	OutputDebugString("HLPFILE_UncompressLZ77 start\n");
#endif

    while (ptr < end)
    {
        int mask = *ptr++;
        for (i = 0; i < 8 && ptr < end; i++, mask >>= 1)
	{
            if (mask & 1)
	    {
                int code   = GET_USHORT(ptr, 0);
                int len    = 3 + (code >> 12);
                int offset = code & 0xfff;
                _fmemcpy(newptr, newptr - offset - 1, len);
                newptr += len;
                ptr    += 2;
	    }
            else *newptr++ = *ptr++;
	}
    }

#ifdef DEBUG
	OutputDebugString("HLPFILE_UncompressLZ77 end\n");
#endif

    return newptr;
}

/***********************************************************************
 *
 *           HLPFILE_UncompressLZ77_Phrases
 */
static BOOL HLPFILE_UncompressLZ77_Phrases(HLPFILE far * hlpfile)
{
    UINT i, num, dec_size;
    BYTE far *buf;
	BYTE far *end;

    if (!HLPFILE_FindSubFile("|Phrases", &buf, &end)) return FALSE;

    num = phrases.num = GET_USHORT(buf, 9);
    if (buf + 2 * num + 0x13 >= end) {//WINE_WARN("1a\n"); 
		return FALSE;
	};

    dec_size = HLPFILE_UncompressedLZ77_Size(buf + 0x13 + 2 * num, end);

    phrases.offsets = (unsigned far *) GlobalAllocPtr(GPTR, sizeof(unsigned) * (num + 1));
    phrases.buffer  = GlobalAllocPtr(GPTR, dec_size);
    if (!phrases.offsets || !phrases.buffer) return FALSE;

    for (i = 0; i <= num; i++)
        phrases.offsets[i] = GET_USHORT(buf, 0x11 + 2 * i) - 2 * num - 2;

    HLPFILE_UncompressLZ77(buf + 0x13 + 2 * num, end, phrases.buffer);

    hlpfile->hasPhrases = TRUE;
    return TRUE;
}

/***********************************************************************
 *
 *           HLPFILE_Uncompress_Phrases40
 */
static BOOL HLPFILE_Uncompress_Phrases40(HLPFILE far * hlpfile)
{
    UINT num, dec_size, cpr_size;
    BYTE far *buf_idx;
	BYTE far *end_idx;
    BYTE far *buf_phs;
	BYTE far *end_phs;
    short i, n;
    long* ptr, mask = 0;
    unsigned short bc;

    if (!HLPFILE_FindSubFile("|PhrIndex", &buf_idx, &end_idx) ||
        !HLPFILE_FindSubFile("|PhrImage", &buf_phs, &end_phs)) return FALSE;

    ptr = (long*)(buf_idx + 9 + 28);
    bc = GET_USHORT(buf_idx, 9 + 24) & 0x0F;
    num = phrases.num = GET_USHORT(buf_idx, 9 + 4);

    //WINE_TRACE("Index: Magic=%08x #entries=%u CpsdSize=%u PhrImgSize=%u\n"
               //"\tPhrImgCprsdSize=%u 0=%u bc=%x ukn=%x\n",
               //GET_UINT(buf_idx, 9 + 0),
               //GET_UINT(buf_idx, 9 + 4),
               //GET_UINT(buf_idx, 9 + 8),
               //GET_UINT(buf_idx, 9 + 12),
               //GET_UINT(buf_idx, 9 + 16),
               //GET_UINT(buf_idx, 9 + 20),
               //GET_USHORT(buf_idx, 9 + 24),
               //GET_USHORT(buf_idx, 9 + 26));

    dec_size = GET_ULONG(buf_idx, 9 + 12);
    cpr_size = GET_ULONG(buf_idx, 9 + 16);

    if (dec_size != cpr_size &&
        dec_size != HLPFILE_UncompressedLZ77_Size(buf_phs + 9, end_phs))
    {
        //WINE_WARN("size mismatch %u %u\n",
                  //dec_size, HLPFILE_UncompressedLZ77_Size(buf_phs + 9, end_phs));
        dec_size = max(dec_size, HLPFILE_UncompressedLZ77_Size(buf_phs + 9, end_phs));
    }

    phrases.offsets = (unsigned far *)GlobalAllocPtr(GPTR, sizeof(unsigned) * (num + 1));
    phrases.buffer  = GlobalAllocPtr(GPTR, dec_size);
    if (!phrases.offsets || !phrases.buffer) return FALSE;

#define getbit() (ptr += (mask < 0), mask = mask*2 + (mask<=0), (*ptr & mask) != 0)

    phrases.offsets[0] = 0;
    for (i = 0; i < num; i++)
    {
        for (n = 1; getbit(); n += 1 << bc);
        if (getbit()) n++;
        if (bc > 1 && getbit()) n += 2;
        if (bc > 2 && getbit()) n += 4;
        if (bc > 3 && getbit()) n += 8;
        if (bc > 4 && getbit()) n += 16;
        phrases.offsets[i + 1] = phrases.offsets[i] + n;
    }
#undef getbit

    if (dec_size == cpr_size)
        _fmemcpy(phrases.buffer, buf_phs + 9, dec_size);
    else
        HLPFILE_UncompressLZ77(buf_phs + 9, end_phs, phrases.buffer);

    hlpfile->hasPhrases = FALSE;
    return TRUE;
}

/***********************************************************************
 *
 *           HLPFILE_Uncompress_Topic
 */
static BOOL HLPFILE_Uncompress_Topic(HLPFILE far * hlpfile)
{
    BYTE far *buf;
	BYTE far *ptr;
	BYTE far *end;
	BYTE far *newptr;
    int  i, newsize = 0;

#ifdef DEBUG
		OutputDebugString("HLPFILE_Uncompress_Topic\n");
#endif

    if (!HLPFILE_FindSubFile("|TOPIC", &buf, &end))
    {
#ifdef DEBUG
		OutputDebugString("topic0\n");
#endif
		return FALSE;
	}

#ifdef DEBUG
		OutputDebugString("HLPFILE_Uncompress_Topic: checking flags\n");
#endif

    switch (hlpfile->flags & (8|4))
    {
    case 8:
#ifdef DEBUG
		OutputDebugString("Unsupported format\n");
#endif
        return FALSE;
    case 4:
#ifdef DEBUG
		OutputDebugString("HLPFILE_Uncompress_Topic: case 4\n");
#endif
        buf += 9;
        topic.wMapLen = (end - buf - 1) / 0x1000 + 1;
        
        for (i = 0; i < topic.wMapLen; i++)
        {
            ptr = buf + i * 0x1000;
            
            /* I don't know why, it's necessary for printman.hlp */
            if (ptr + 0x44 > end) ptr = end - 0x44;

            newsize += HLPFILE_UncompressedLZ77_Size(ptr + 0xc, min(end, ptr + 0x1000));
        }

#ifdef DEBUG
		OutputDebugString("HLPFILE_Uncompress_Topic: Allocate map index\n");
#endif
        
        topic.map = (BYTE far * far *)GlobalAllocPtr(GPTR,
                              topic.wMapLen * sizeof(topic.map[0]) + newsize);
        if (!topic.map) return FALSE;

#ifdef DEBUG
		OutputDebugString("HLPFILE_Uncompress_Topic: Move pointers\n");
#endif

        newptr = (char far *)(topic.map + topic.wMapLen);
        topic.end = newptr + newsize;

#ifdef DEBUG
		OutputDebugString("HLPFILE_Uncompress_Topic: Uncompress\n");
#endif

        for (i = 0; i < topic.wMapLen; i++)
        {
            ptr = buf + i * 0x1000;
            if (ptr + 0x44 > end) ptr = end - 0x44;
            topic.map[i] = newptr;

#ifdef DEBUG
			OutputDebugString("HLPFILE_Uncompress_Topic: Uncompress block\n");
#endif
            newptr = HLPFILE_UncompressLZ77(ptr + 0xc, min(end, ptr + 0x1000), newptr);
        }
        break;
    case 0:
#ifdef DEBUG
		OutputDebugString("HLPFILE_Uncompress_Topic: case 0\n");
#endif
        /* basically, we need to copy the 0x1000 byte pages (removing the first 0x0C) in
         * one single are in memory
         */
#define DST_LEN (0x1000 - 0x0C)
        buf += 9;
        newsize = end - buf;
        /* number of destination pages */
        topic.wMapLen = (newsize - 1) / DST_LEN + 1;
        topic.map = (BYTE far * far *)GlobalAllocPtr(GPTR,
                              topic.wMapLen * (sizeof(topic.map[0]) + DST_LEN));
        if (!topic.map) return FALSE;
        newptr = (char far *)(topic.map + topic.wMapLen);
        topic.end = newptr + newsize;

        for (i = 0; i < topic.wMapLen; i++)
        {
            topic.map[i] = newptr + i * DST_LEN;
            _fmemcpy(topic.map[i], buf + i * 0x1000 + 0x0C, DST_LEN);
        }
#undef DST_LEN
        break;
    }
    return TRUE;
}

/***********************************************************************
 *
 *           HLPFILE_Uncompress2
 */

static void HLPFILE_Uncompress2(const BYTE far *ptr, const BYTE far * end, BYTE far * newptr, const BYTE far * newend)
{
    BYTE far *phptr;
	BYTE far *phend;
    UINT code;
    UINT index;

    while (ptr < end && newptr < newend)
    {
        if (!*ptr || *ptr >= 0x10)
            *newptr++ = *ptr++;
        else
	{
            code  = 0x100 * ptr[0] + ptr[1];
            index = (code - 0x100) / 2;

            phptr = phrases.buffer + phrases.offsets[index];
            phend = phrases.buffer + phrases.offsets[index + 1];

            if (newptr + (phend - phptr) > newend)
            {
          //      WINE_FIXME("buffer overflow %p > %p for %d bytes\n", 
            //               newptr, newend, phend - phptr);
                return;
            }
            _fmemcpy(newptr, phptr, phend - phptr);
            newptr += phend - phptr;
            if (code & 1) *newptr++ = ' ';

            ptr += 2;
	}
    }
    if (newptr > newend) {};//WINE_FIXME("buffer overflow %p > %p\n", newptr, newend);
}

/******************************************************************
 *		HLPFILE_Uncompress3
 *
 *
 */
static BOOL HLPFILE_Uncompress3(char far * dst, const char far * dst_end,
                                const BYTE far * src, const BYTE far * src_end)
{
    int         idx, len;

    for (; src < src_end; src++)
    {
        if ((*src & 1) == 0)
        {
            idx = *src / 2;
            if (idx > phrases.num) 
            {
                //WINE_ERR("index in phrases %d/%d\n", idx, phrases.num);
                len = 0;
            }
            else 
            {
                len = phrases.offsets[idx + 1] - phrases.offsets[idx];
                if (dst + len <= dst_end)
                    _fmemcpy(dst, &phrases.buffer[phrases.offsets[idx]], len);
            }
        }
        else if ((*src & 0x03) == 0x01)
        {
            idx = (*src + 1) * 64;
            idx += *++src;
            if (idx > phrases.num) 
            {
                //WINE_ERR("index in phrases %d/%d\n", idx, phrases.num);
                len = 0;
            }
            else
            {
                len = phrases.offsets[idx + 1] - phrases.offsets[idx];
                if (dst + len <= dst_end)
                    _fmemcpy(dst, &phrases.buffer[phrases.offsets[idx]], len);
            }
        }
        else if ((*src & 0x07) == 0x03)
        {
            len = (*src / 8) + 1;
            if (dst + len <= dst_end)
                _fmemcpy(dst, src + 1, len);
            src += len;
        }
        else
        {
            len = (*src / 16) + 1;
            if (dst + len <= dst_end)
                _fmemset(dst, ((*src & 0x0F) == 0x07) ? ' ' : 0, len);
        }
        dst += len;
    }

    if (dst > dst_end) {}//WINE_ERR("buffer overflow (%p > %p)\n", dst, dst_end);
    return TRUE;
}

/******************************************************************
 *		HLPFILE_UncompressRLE
 *
 *
 */
static void HLPFILE_UncompressRLE(const BYTE far * src, const BYTE far * end, BYTE far * far * dst, unsigned dstsz)
{
    BYTE        ch;
    BYTE far *  sdst = *dst + dstsz;

    while (src < end)
    {
        ch = *src++;
        if (!(ch & 0x7F)) continue;
        if (ch & 0x80)
        {
            ch &= 0x7F;
            if ((*dst) + ch < sdst)
                _fmemcpy(*dst, src, ch);
            src += ch;
        }
        else
        {
            if ((*dst) + ch < sdst)
                _fmemset(*dst, (char)*src++, ch);
        }
        *dst += ch;
    }
    if (*dst != sdst)
	{}//WINE_WARN("Buffer X-flow: d(%u) instead of d(%u)\n",
        //          *dst - (sdst - dstsz), dstsz);
}

/******************************************************************
 *		HLPFILE_EnumBTreeLeaves
 *
 *
 */
static void HLPFILE_EnumBTreeLeaves(const BYTE far * buf, const BYTE far * end, unsigned (*fn)(const BYTE far *, void far *), void far * user)
{
    unsigned    psize, pnext;
    unsigned    num, nlvl;
    const BYTE far * ptr;

    num    = GET_ULONG(buf, 9 + 34);
    psize  = GET_USHORT(buf, 9 + 4);
    nlvl   = GET_USHORT(buf, 9 + 32);
    pnext  = GET_USHORT(buf, 9 + 26);

    //WINE_TRACE("BTree: #entries=%u pagSize=%u #levels=%u #pages=%u root=%u struct%16s\n",
               //num, psize, nlvl, GET_USHORT(buf, 9 + 30), pnext, buf + 9 + 6);
    if (!num) return;

    while (--nlvl > 0)
    {
        ptr = (buf + 9 + 38) + pnext * psize;
        //WINE_TRACE("BTree: (index[%u]) unused=%u #entries=%u <%u\n",
          //         pnext, GET_USHORT(ptr, 0), GET_USHORT(ptr, 2), GET_USHORT(ptr, 4));
        pnext = GET_USHORT(ptr, 4);
    }
    while (pnext != 0xFFFF)
    {
        const BYTE far *     node_page;
        unsigned short  limit;

        node_page = ptr = (buf + 9 + 38) + pnext * psize;
        limit = GET_USHORT(ptr, 2);
        //WINE_TRACE("BTree: (leaf [%u]) unused=%u #entries=%u <%u >%u\n",
          //         pnext, GET_USHORT(ptr, 0), limit, GET_USHORT(ptr, 4), GET_USHORT(ptr, 6));
        ptr += 8;
        while (limit--)
            ptr += (fn)(ptr, user);
        pnext = GET_USHORT(node_page, 6);
    }
}

struct myfncb {
    HLPFILE far * hlpfile;
    int         i;
};

static unsigned myfn(const BYTE far * ptr, void far * user)
{
    struct myfncb far *      m = user;

    m->hlpfile->Context[m->i].lHash  = GET_ULONG(ptr, 0);
    m->hlpfile->Context[m->i].offset = GET_ULONG(ptr, 4);
    m->i++;
    return 8;
}

/***********************************************************************
 *
 *           HLPFILE_GetContext
 */
static BOOL HLPFILE_GetContext(HLPFILE far *hlpfile)
{
    BYTE far *          cbuf;
	BYTE far *          cend;
    struct myfncb       m;
    unsigned            clen;

    if (!HLPFILE_FindSubFile("|CONTEXT",  &cbuf, &cend)) {//WINE_WARN("context0\n"); 
		return FALSE;
	}

    clen = GET_ULONG(cbuf, 0x2b);
    hlpfile->Context = (HLPFILE_CONTEXT far *)GlobalAllocPtr(GPTR, clen * sizeof(HLPFILE_CONTEXT));
    if (!hlpfile->Context) return FALSE;
    hlpfile->wContextLen = clen;

    m.hlpfile = hlpfile;
    m.i = 0;
    HLPFILE_EnumBTreeLeaves(cbuf, cend, myfn, &m);

    return TRUE;
}

/******************************************************************
 *		HLPFILE_DeleteLink
 *
 *
 */
void HLPFILE_FreeLink(HLPFILE_LINK far * link)
{
    if (link && !--link->wRefCount)
        GlobalFreePtr(link);
}

/***********************************************************************
 *
 *           HLPFILE_DeleteParagraph
 */
static void HLPFILE_DeleteParagraph(HLPFILE_PARAGRAPH far * paragraph)
{
    HLPFILE_PARAGRAPH far * next;

    while (paragraph)
    {
        next = paragraph->next;

        if (paragraph->cookie == para_metafile)
            DeleteMetaFile(paragraph->u.gfx.u.mf.hMetaFile);

        HLPFILE_FreeLink(paragraph->link);

        GlobalFreePtr(paragraph);
        paragraph = next;
    }
}

/***********************************************************************
 *
 *           DeleteMacro
 */
static void HLPFILE_DeleteMacro(HLPFILE_MACRO far * macro)
{
    HLPFILE_MACRO far *      next;

    while (macro)
    {
        next = macro->next;
        GlobalFreePtr(macro);
        macro = next;
    }
}

/***********************************************************************
 *
 *           DeletePage
 */
static void HLPFILE_DeletePage(HLPFILE_PAGE far * page)
{
    HLPFILE_PAGE far * next;

    while (page)
    {
        next = page->next;
        HLPFILE_DeleteParagraph(page->first_paragraph);
        HLPFILE_DeleteMacro(page->first_macro);
        GlobalFreePtr(page);
        page = next;
    }
}

/***********************************************************************
 *
 *           HLPFILE_FreeHlpFile
 */
void HLPFILE_FreeHlpFile(HLPFILE far * hlpfile)
{
    unsigned i;

    if (!hlpfile || --hlpfile->wRefCount > 0) return;

    if (hlpfile->next) hlpfile->next->prev = hlpfile->prev;
    if (hlpfile->prev) hlpfile->prev->next = hlpfile->next;
    else first_hlpfile = hlpfile->next;

    if (hlpfile->numFonts)
    {
        for (i = 0; i < hlpfile->numFonts; i++)
        {
            DeleteObject(hlpfile->fonts[i].hFont);
        }
        GlobalFreePtr(hlpfile->fonts);
    }

    if (hlpfile->numBmps)
    {
        for (i = 0; i < hlpfile->numBmps; i++)
        {
            DeleteObject(hlpfile->bmps[i]);
        }
        GlobalFreePtr(hlpfile->bmps);
    }

    HLPFILE_DeletePage(hlpfile->first_page);
    HLPFILE_DeleteMacro(hlpfile->first_macro);

    if (hlpfile->numWindows)    GlobalFreePtr(hlpfile->windows);
    if (hlpfile->Context)       GlobalFreePtr(hlpfile->Context);
    if (hlpfile->lpszTitle)     GlobalFreePtr(hlpfile->lpszTitle);
    if (hlpfile->lpszCopyright) GlobalFreePtr(hlpfile->lpszCopyright);
    GlobalFreePtr(hlpfile);
}
