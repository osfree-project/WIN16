/*
 * Program Manager
 *
 * Copyright 1996 Ulrich Schmid
 *           1997 Peter Schlaile
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

#include "win16.h"
#include "progman.h"

#include "grpfile.h"

#define MALLOCHUNK 1000

#define GET_USHORT(buffer, i)\
  (((BYTE)((buffer)[(i)]) + 0x100 * (BYTE)((buffer)[(i)+1])))
#define GET_SHORT(buffer, i)\
  (((BYTE)((buffer)[(i)]) + 0x100 * (signed char)((buffer)[(i)+1])))
#define PUT_SHORT(buffer, i, s)\
  (((buffer)[(i)] = (s) & 0xff, (buffer)[(i)+1] = ((s) >> 8) & 0xff))

static BOOL GRPFILE_ReadFileToBuffer(LPCSTR path, HLOCAL *phBuffer,
				     int *piSize);
static HLOCAL GRPFILE_ScanGroup(LPCSTR, int, LPCSTR/*, BOOL*/);
static HLOCAL GRPFILE_ScanProgram(LPCSTR, int, LPCSTR, int,
                                  LPCSTR, HLOCAL,LPCSTR, BOOL);
static BOOL GRPFILE_DoWriteGroupFile(HFILE file, PROGGROUP *group);


/***********************************************************************
 *
 *           GRPFILE_ReadGroupFile
 */

HLOCAL GRPFILE_ReadGroupFile(LPCSTR lpszPath)
{
  char   szPath_gr[MAX_PATHNAME_LEN];
  OFSTRUCT dummy;
  HLOCAL hBuffer, hGroup;
  int    size;

  /* Read the whole file into a buffer */
  if (!GRPFILE_ReadFileToBuffer(lpszPath, &hBuffer, &size))
    {
      MAIN_MessageBoxIDS_s(IDS_FILE_READ_ERROR_s, lpszPath, IDS_ERROR, MB_OK);
      return(0);
    }

  /* Interpret buffer */
  hGroup = GRPFILE_ScanGroup(LocalLock(hBuffer), size,
			     lpszPath);
  if (!hGroup)
    MAIN_MessageBoxIDS_s(IDS_GRPFILE_READ_ERROR_s, lpszPath, IDS_ERROR, MB_OK);

  LocalFree(hBuffer);

  return(hGroup);
}

/***********************************************************************
 *
 *           GRPFILE_ReadFileToBuffer
 */

static BOOL GRPFILE_ReadFileToBuffer(LPCSTR path, HLOCAL *phBuffer,
				     int *piSize)
{
  UINT   len, size;
  LPSTR  buffer;
  HLOCAL hBuffer, hNewBuffer;
  HFILE  file;

  file=_lopen(path, OF_READ);
  if (file == HFILE_ERROR) return FALSE;

  size = 0;
  hBuffer = LocalAlloc(LMEM_FIXED, MALLOCHUNK + 1);
  if (!hBuffer) return FALSE;
  buffer = LocalLock(hBuffer);

  // @todo Why allocate by chunk? Is is better read whole file???
  while ((len = _lread(file, buffer + size, MALLOCHUNK))
         == MALLOCHUNK)
    {
      size += len;
      hNewBuffer = LocalReAlloc(hBuffer, size + MALLOCHUNK + 1,
				LMEM_MOVEABLE);
      if (!hNewBuffer)
	{
	  LocalFree(hBuffer);
	  return FALSE;
	}
      hBuffer = hNewBuffer;
      buffer = LocalLock(hBuffer);
  }

  _lclose(file);

  if (len == (UINT)HFILE_ERROR)
    {
      LocalFree(hBuffer);
      return FALSE;
    }
	
  size += len;
  buffer[size] = 0;  // Why add zero to end of buffer???

  *phBuffer = hBuffer;
  *piSize   = size;
  return TRUE;
}

/***********************************************************************
 *           GRPFILE_ScanGroup
 */

static HLOCAL GRPFILE_ScanGroup(LPCSTR buffer, int size,
                                LPCSTR lpszGrpFile)
{
  HLOCAL  hGroup;
  int     i, seqnum;
  LPCSTR  extension;
  LPCSTR  lpszName;
  int     x, y, width, height, iconx, icony, nCmdShow;
  int     number_of_programs;
  struct tagGROUPHEADER * header;
  WORD    wCurLogPixelsX, wCurLogPixelsY;
  BOOL    bRebuildIcons;		// TRUE if stored in GRP icons need to be rebuilded for current display
#ifdef DEBUG
  char DebugBuffer[200];
#endif
  if (buffer[0] != 'P' || buffer[1] != 'M') return(0);
  if (buffer[2] == 'C' && buffer[3] == 'C') {}
  else return(0);

  header=(struct tagGROUPHEADER *)buffer;

#ifdef DEBUG
  OutputDebugString("Group header\n\r");
  OutputDebugString("============\n\r");
  wsprintf(DebugBuffer, "Magic:\t%c%c%c%c\n\r", header->cIdentifier[0],header->cIdentifier[1],header->cIdentifier[2],header->cIdentifier[3]);
  OutputDebugString(DebugBuffer);
  wsprintf(DebugBuffer, "Checksum addenum:\t%04x\n\r", header->wCheckSum);
  OutputDebugString(DebugBuffer);
  wsprintf(DebugBuffer, "Size without TAGDATA:\t%04x\n\r", header->cbGroup);
  OutputDebugString(DebugBuffer);
  wsprintf(DebugBuffer, "SW flags:\t%04x\n\r", header->nCmdShow);
  OutputDebugString(DebugBuffer);
//  RECT  rcNormal;		// Coordinates of the group window (the window in which the group icons appear). It is a rectangular structure.
//  POINT ptMin;			// Coordinate of the lower-left corner of the group window with respect to the parent window. It is a point structure.
  wsprintf(DebugBuffer, "Group name offset:\t%04x\n\r", header->pName);
  OutputDebugString(DebugBuffer);
  wsprintf(DebugBuffer, "Target display X resolution:\t%04x\n\r", header->wLogPixelsX);
  OutputDebugString(DebugBuffer);
  wsprintf(DebugBuffer, "Target display Y resolution:\t%04x\n\r", header->wLogPixelsY);
  OutputDebugString(DebugBuffer);
  wsprintf(DebugBuffer, "Current display X resolution:\t%04x\n", Globals.wLogPixelsX);
  OutputDebugString(DebugBuffer);
  wsprintf(DebugBuffer, "Current display Y resolution:\t%04x\n", Globals.wLogPixelsY);
  OutputDebugString(DebugBuffer);
  wsprintf(DebugBuffer, "Target bits per pixel:\t%01x\n\r", header->bBitsPerPixel);
  OutputDebugString(DebugBuffer);
  wsprintf(DebugBuffer, "Target planes:\t%01x\n\r", header->bPlanes);
  OutputDebugString(DebugBuffer);
  wsprintf(DebugBuffer, "Current bits per pixel:\t%01x\n\r", Globals.bBitsPixel);
  OutputDebugString(DebugBuffer);
  wsprintf(DebugBuffer, "Current planes:\t%01x\n\r", Globals.bPlanes);
  OutputDebugString(DebugBuffer);
//  WORD  wReserved;		// Must be 0
  wsprintf(DebugBuffer, "Number of ITEMDATA pointers(?):\t%d\n", header->cItems);
  OutputDebugString(DebugBuffer);
#endif

  /* checksum = GET_USHORT(buffer, 4)   (ignored) */

  //extension = buffer + header->cbGroup; //GET_USHORT(buffer, 6);
  extension = buffer + GET_USHORT(buffer, 6);
  if (extension == buffer + size) extension = 0;
  else if (extension + 6 > buffer + size) return(0);

  nCmdShow = header->nCmdShow; //GET_USHORT(buffer,  8);
  x        = header->rcNormal.left;//GET_SHORT(buffer,  10);
  y        = header->rcNormal.top;//GET_SHORT(buffer,  12);
  width    = header->rcNormal.right;//GET_USHORT(buffer, 14);
  height   = header->rcNormal.bottom;//GET_USHORT(buffer, 16);
  iconx    = header->ptMin.x;//GET_SHORT(buffer,  18);
  icony    = header->ptMin.y;//GET_SHORT(buffer,  20);
  lpszName = buffer + header->pName;//GET_USHORT(buffer, 22);

  if (lpszName >= buffer + size) return(0);

  bRebuildIcons = (header->wLogPixelsX != Globals.wLogPixelsX) |
				(header->wLogPixelsY != Globals.wLogPixelsY) |
				(header->bBitsPerPixel != Globals.bBitsPixel) |
				(header->bPlanes != Globals.bPlanes);

  /* unknown bytes 24 - 31 ignored */
  /*
    Unknown bytes should be:
     = GET_SHORT(buffer, 24);
    wLogPixelsY = GET_SHORT(buffer, 26);
    byBitsPerPixel = byte at 28;
    byPlanes     = byte at 29;
    wReserved   = GET_SHORT(buffer, 30);
    */

  hGroup = GROUP_AddGroup(lpszName, lpszGrpFile, nCmdShow, x, y,
                          width, height, iconx, icony,
                          TRUE);
  if (!hGroup) return(0);

  //number_of_programs = header->cItems;//GET_USHORT(buffer, 32);
  number_of_programs = header->cItems;//GET_USHORT(buffer, 32);
  if (2 * number_of_programs + 34 > size) return(0);
  //if (2 * number_of_programs + 34 > size)
  //{
  //!!! THis is number of items in array not number of programs in group!
  //    MessageBox(Globals.hMainWnd, "1", "debug", MB_YESNO);
  //    return(0);
  //}
  for (i=0, seqnum=0; i < number_of_programs; i++, seqnum++)
    {
      LPCSTR program_ptr = buffer + GET_USHORT(buffer, 34 + 2*i);
      if (program_ptr + 24 > buffer + size) return(0);
      if (!GET_USHORT(buffer, 34 + 2*i)) continue;
      if (!GRPFILE_ScanProgram(buffer, size, program_ptr, seqnum,
                               extension, hGroup, lpszGrpFile, bRebuildIcons))
        {
          GROUP_DeleteGroup(hGroup);
          return(0);
        }
    }


  /* FIXME shouldn't be necessary */
  GROUP_ShowGroupWindow(hGroup);

  return hGroup;
}

/***********************************************************************
 *           GRPFILE_ScanProgram
 */

static HLOCAL GRPFILE_ScanProgram(LPCSTR buffer, int size,
                                  LPCSTR program_ptr, int seqnum,
                                  LPCSTR extension, HLOCAL hGroup,
                                  LPCSTR lpszGrpFile, BOOL bRebuildIcons)
{
  int    icontype;
  HICON  hIcon;
  LPCSTR lpszName, lpszCmdLine, lpszIconFile, lpszWorkDir;
  LPCSTR iconinfo_ptr, iconANDbits_ptr, iconXORbits_ptr;
  int    x, y, nIconIndex, iconANDsize, iconXORsize;
  int    nHotKey, nCmdShow;
  CURSORICONINFO iconinfo;
  struct tagITEMDATA * itemdata;

  itemdata = (struct tagITEMDATA *)program_ptr;

  x               = GET_SHORT(program_ptr, 0);
  y               = GET_SHORT(program_ptr, 2);
  nIconIndex      = itemdata->iIcon; //GET_USHORT(program_ptr, 4);

  /* FIXME is this correct ?? */
  // @todo no. It is incorrect. This is seems to be size of buffer for getting resource from file or buffer size for icon creation.
  // Logic is following: If bRebuildIcons is TRUE (means icons in GRP not same as for current display),
  // then get icons from resources and store them in GRP file. If FALSE then reuse icons from GRP file)
  // @todo here is a some problem on modern systems. Because screen resolution too high GRP file size
  // exceed 64Kb. As result, no memory info. May be solution is to use huge pointers?
  
  icontype = itemdata->cbResource; //GET_USHORT(program_ptr,  6);		// cbresource    specifies the count of bytes in the icon resource, which appears in the executable file for the application. 
  switch (icontype)
    {
    default:
      MAIN_MessageBoxIDS_s(IDS_UNKNOWN_FEATURE_s, lpszGrpFile,
                           IDS_WARNING, MB_OK);
    case 0x048c:
	// @todo Seems totally incorrect here. Most probably depend on Plane. And not clear why XOR and AND swapped
      iconXORsize     = itemdata->cbANDPlane; //GET_USHORT(program_ptr,  8);
      iconANDsize     = itemdata->cbXORPlane; //GET_USHORT(program_ptr, 10) / 8; 
      iconinfo_ptr    = buffer + itemdata->pHeader; //GET_USHORT(program_ptr, 12);
      iconXORbits_ptr = buffer + itemdata->pANDPlane; //GET_USHORT(program_ptr, 14);
      iconANDbits_ptr = buffer + itemdata->pXORPlane; //GET_USHORT(program_ptr, 16);
      iconinfo.ptHotSpot.x   = GET_USHORT(iconinfo_ptr, 0);
      iconinfo.ptHotSpot.y   = GET_USHORT(iconinfo_ptr, 2);
      iconinfo.nWidth        = GET_USHORT(iconinfo_ptr, 4);
      iconinfo.nHeight       = GET_USHORT(iconinfo_ptr, 6);
      iconinfo.nWidthBytes   = GET_USHORT(iconinfo_ptr, 8);
      iconinfo.bPlanes       = GET_USHORT(iconinfo_ptr, 10);
      iconinfo.bBitsPerPixel = GET_USHORT(iconinfo_ptr, 11);
      break;
    case 0x000c:
      iconANDsize     = itemdata->cbANDPlane; // GET_USHORT(program_ptr,  8);
      iconXORsize     = itemdata->cbXORPlane; // GET_USHORT(program_ptr, 10);
      iconinfo_ptr    = buffer + itemdata->pHeader; //GET_USHORT(program_ptr, 12);
      iconANDbits_ptr = buffer + itemdata->pANDPlane; //GET_USHORT(program_ptr, 14);
      iconXORbits_ptr = buffer + itemdata->pXORPlane; //GET_USHORT(program_ptr, 16);
      iconinfo.ptHotSpot.x   = GET_USHORT(iconinfo_ptr, 0);
      iconinfo.ptHotSpot.y   = GET_USHORT(iconinfo_ptr, 2);
      iconinfo.nWidth        = GET_USHORT(iconinfo_ptr, 4);
      iconinfo.nHeight       = GET_USHORT(iconinfo_ptr, 6);
      iconinfo.nWidthBytes = GET_USHORT(iconinfo_ptr, 8) * 8;
      iconinfo.bPlanes       = GET_USHORT(iconinfo_ptr, 10);
      iconinfo.bBitsPerPixel = GET_USHORT(iconinfo_ptr, 11);
    }

  if (iconANDbits_ptr + iconANDsize > buffer + size ||
      iconXORbits_ptr + iconXORsize > buffer + size) return(0);


  hIcon = CreateIcon( Globals.hInstance, iconinfo.nWidth, iconinfo.nHeight,
                      iconinfo.bPlanes, iconinfo.bBitsPerPixel,
                      iconANDbits_ptr, iconXORbits_ptr );

  lpszName        = buffer + GET_USHORT(program_ptr, 18);
  lpszCmdLine     = buffer + GET_USHORT(program_ptr, 20);
  lpszIconFile    = buffer + GET_USHORT(program_ptr, 22);
  if (iconinfo_ptr + 6 > buffer + size ||
      lpszName         > buffer + size ||
      lpszCmdLine      > buffer + size ||
      lpszIconFile     > buffer + size) return(0);

  /* Scan Extensions */
  lpszWorkDir = "";
  nHotKey     = 0;
  nCmdShow    = SW_SHOWNORMAL;
  if (extension)
    {
      LPCSTR ptr = extension;
      while (ptr + 6 <= buffer + size)
        {
          UINT type   = GET_USHORT(ptr, 0);
          UINT number = GET_USHORT(ptr, 2);
          UINT skip   = GET_USHORT(ptr, 4);

          if (number == seqnum)
            {
              switch (type)
                {
                case 0x8000:
                  if (ptr + 10 > buffer + size) return(0);
                  if (ptr[6] != 'P' || ptr[7] != 'M' ||
                      ptr[8] != 'C' || ptr[9] != 'C') return(0);
                  break;
                case 0x8101:
                  lpszWorkDir = ptr + 6;
                  break;
                case 0x8102:
                  if (ptr + 8 > buffer + size) return(0);
                  nHotKey = GET_USHORT(ptr, 6);
                  break;
                case 0x8103:
                  if (ptr + 8 > buffer + size) return(0);
                  nCmdShow = GET_USHORT(ptr, 6);
                  break;
                default:
                  MAIN_MessageBoxIDS_s(IDS_UNKNOWN_FEATURE_s,
                                       lpszGrpFile, IDS_WARNING, MB_OK);
                }
            }
          if (!skip) break;
          ptr += skip;
        }
    }

  return (PROGRAM_AddProgram(hGroup, hIcon, lpszName, x, y,
                             lpszCmdLine, lpszIconFile,
                             nIconIndex, lpszWorkDir,
                             nHotKey, nCmdShow));
}

/***********************************************************************
 *
 *           GRPFILE_WriteGroupFile
 */

BOOL GRPFILE_WriteGroupFile(HLOCAL hGroup)
{
  PROGGROUP *group = (PROGGROUP *)LocalLock(hGroup);
  OFSTRUCT dummy;
  HFILE file;
  BOOL ret;

  /* Open file */
  file = _lcreat(LocalLock(group->hGrpFile), 0);
  if (file != HFILE_ERROR)
    {
      ret = GRPFILE_DoWriteGroupFile(file, group);
      _lclose(file);
    }
  else ret = FALSE;

  if (!ret)
    MAIN_MessageBoxIDS_s(IDS_FILE_WRITE_ERROR_s, LocalLock(group->hGrpFile), IDS_ERROR, MB_OK);

  return(ret);
}

/***********************************************************************
 *
 *           GRPFILE_CalculateSizes
 */

static VOID GRPFILE_CalculateSizes(PROGRAM *program,
                                   int *Progs, int *Icons)
{
  CURSORICONINFO *iconinfo = (CURSORICONINFO *)LocalLock(program->hIcon);
  int sizeXor = iconinfo->nHeight * iconinfo->nWidthBytes;
  int sizeAnd = iconinfo->nHeight * ((iconinfo->nWidth + 15) / 16 * 2);

  *Progs += 24;
  *Progs += lstrlen(LocalLock(program->hName)) + 1;
  *Progs += lstrlen(LocalLock(program->hCmdLine)) + 1;
  *Progs += lstrlen(LocalLock(program->hIconFile)) + 1;

  *Icons += 12; /* IconInfo */
  *Icons += sizeAnd;
  *Icons += sizeXor;
}

/***********************************************************************/
unsigned int GRPFILE_checksum;
/***********************************************************************
 *
 *           GRPFILE_InitChecksum
 */

static void GRPFILE_InitChecksum(void)
{
        GRPFILE_checksum = 0;
}

/***********************************************************************
 *
 *           GRPFILE_GetChecksum
 */

static unsigned int GRPFILE_GetChecksum(void)
{
        return GRPFILE_checksum;
}


/***********************************************************************
 *
 *           GRPFILE_DoWriteGroupFile
 */

static BOOL GRPFILE_DoWriteGroupFile(HFILE file, PROGGROUP *group)
{
  char buffer[34];
  HLOCAL hProgram;
  int    NumProg, Title, Progs, Icons, Extension;
  int    CurrProg, CurrIcon, nCmdShow, ptr, seqnum;
  BOOL   need_extension;
  LPCSTR lpszTitle = LocalLock(group->hName);

  unsigned int checksum;

  GRPFILE_InitChecksum();

  /* Calculate offsets */
  NumProg = 0;
  Icons   = 0;
  Extension = 0;
  need_extension = FALSE;
  hProgram = group->hPrograms;
  while(hProgram)
    {
      PROGRAM *program = (PROGRAM *)LocalLock(hProgram);
      LPCSTR lpszWorkDir = LocalLock(program->hWorkDir);

      NumProg++;
      GRPFILE_CalculateSizes(program, &Icons, &Extension);

      /* Set a flag if an extension is needed */
      if (lpszWorkDir[0] || program->nHotKey ||
          program->nCmdShow != SW_SHOWNORMAL) need_extension = TRUE;

      hProgram = program->hNext;
    }
  Title      = 34 + NumProg * 2;
  Progs      = Title + lstrlen(lpszTitle) + 1;
  Icons     += Progs;
  Extension += Icons;

  /* Header */
  buffer[0] = 'P';
  buffer[1] = 'M';
  buffer[2] = 'C';
  buffer[3] = 'C';

  PUT_SHORT(buffer,  4, 0); /* Checksum zero for now, written later */
  PUT_SHORT(buffer,  6, Extension);
  /* Update group->nCmdShow */
  if (IsIconic(group->hWnd))      nCmdShow = SW_SHOWMINIMIZED;
  else if (IsZoomed(group->hWnd)) nCmdShow = SW_SHOWMAXIMIZED;
  else                            nCmdShow = SW_SHOWNORMAL;
  PUT_SHORT(buffer,  8, nCmdShow);
  PUT_SHORT(buffer, 10, group->x);
  PUT_SHORT(buffer, 12, group->y);
  PUT_SHORT(buffer, 14, group->width);
  PUT_SHORT(buffer, 16, group->height);
  PUT_SHORT(buffer, 18, group->iconx);
  PUT_SHORT(buffer, 20, group->icony);
  PUT_SHORT(buffer, 22, Title);
  PUT_SHORT(buffer, 24, 0x0020); /* unknown */
  PUT_SHORT(buffer, 26, 0x0020); /* unknown */
  PUT_SHORT(buffer, 28, 0x0108); /* unknown */
  PUT_SHORT(buffer, 30, 0x0000); /* unknown */
  PUT_SHORT(buffer, 32, NumProg);

  if ((UINT)HFILE_ERROR == _lwrite(file, buffer, 34)) return FALSE;

  /* Program table */
  CurrProg = Progs;
  CurrIcon = Icons;
  hProgram = group->hPrograms;
  while(hProgram)
    {
      PROGRAM *program = (PROGRAM *)LocalLock(hProgram);

      PUT_SHORT(buffer, 0, CurrProg);
      if ((UINT)HFILE_ERROR == _lwrite(file, buffer, 2))
              return FALSE;

      GRPFILE_CalculateSizes(program, &CurrProg, &CurrIcon);
      hProgram = program->hNext;
    }

  /* Title */
  if ((UINT)HFILE_ERROR == _lwrite(file, lpszTitle,
                                               lstrlen(lpszTitle) + 1))
    return FALSE;

  /* Program entries */
  CurrProg = Progs;
  CurrIcon = Icons;
  hProgram = group->hPrograms;
  while(hProgram)
    {
      PROGRAM *program = (PROGRAM *)LocalLock(hProgram);
      CURSORICONINFO *iconinfo = (CURSORICONINFO *)LocalLock(program->hIcon);
      LPCSTR Name     = LocalLock(program->hName);
      LPCSTR CmdLine  = LocalLock(program->hCmdLine);
      LPCSTR IconFile = LocalLock(program->hIconFile);
      int sizeXor = iconinfo->nHeight * iconinfo->nWidthBytes;
      int sizeAnd = iconinfo->nHeight * ((iconinfo->nWidth + 15) / 16 * 2);

      PUT_SHORT(buffer,  0, program->x);
      PUT_SHORT(buffer,  2, program->y);
      PUT_SHORT(buffer,  4, program->nIconIndex);
      PUT_SHORT(buffer,  6, 0x048c);            /* unknown */
      PUT_SHORT(buffer,  8, sizeXor);
      PUT_SHORT(buffer, 10, sizeAnd * 8);
      PUT_SHORT(buffer, 12, CurrIcon);
      PUT_SHORT(buffer, 14, CurrIcon + 12 + sizeAnd);
      PUT_SHORT(buffer, 16, CurrIcon + 12);
      ptr = CurrProg + 24;
      PUT_SHORT(buffer, 18, ptr);
      ptr += lstrlen(Name) + 1;
      PUT_SHORT(buffer, 20, ptr);
      ptr += lstrlen(CmdLine) + 1;
      PUT_SHORT(buffer, 22, ptr);

      if ((UINT)HFILE_ERROR == _lwrite(file, buffer, 24) ||
          (UINT)HFILE_ERROR == _lwrite(file, Name, lstrlen(Name) + 1) ||
          (UINT)HFILE_ERROR == _lwrite(file, CmdLine, lstrlen(CmdLine) + 1) ||
          (UINT)HFILE_ERROR == _lwrite(file, IconFile, lstrlen(IconFile) + 1))
        return FALSE;

      GRPFILE_CalculateSizes(program, &CurrProg, &CurrIcon);
      hProgram = program->hNext;
    }

  /* Icons */
#if 0  /* FIXME: this is broken anyway */
  hProgram = group->hPrograms;
  while(hProgram)
    {
      PROGRAM *program = LocalLock(hProgram);
      CURSORICONINFO *iconinfo = LocalLock(program->hIcon);
      LPVOID XorBits, AndBits;
      int sizeXor = iconinfo->nHeight * iconinfo->nWidthBytes;
      int sizeAnd = iconinfo->nHeight * ((iconinfo->nWidth + 15) / 16 * 2);
      /* DumpIcon16(LocalLock(program->hIcon), 0, &XorBits, &AndBits);*/

      PUT_SHORT(buffer, 0, iconinfo->ptHotSpot.x);
      PUT_SHORT(buffer, 2, iconinfo->ptHotSpot.y);
      PUT_SHORT(buffer, 4, iconinfo->nWidth);
      PUT_SHORT(buffer, 6, iconinfo->nHeight);
      PUT_SHORT(buffer, 8, iconinfo->nWidthBytes);
      buffer[10] = iconinfo->bPlanes;
      buffer[11] = iconinfo->bBitsPerPixel;

      if ((UINT)HFILE_ERROR == _lwrite(file, buffer, 12) ||
          (UINT)HFILE_ERROR == _lwrite(file, AndBits, sizeAnd) ||
          (UINT)HFILE_ERROR == _lwrite(file, XorBits, sizeXor)) return FALSE;

      hProgram = program->hNext;
    }
#endif

  if (need_extension)
    {
      /* write `PMCC' extension */
      PUT_SHORT(buffer, 0, 0x8000);
      PUT_SHORT(buffer, 2, 0xffff);
      PUT_SHORT(buffer, 4, 0x000a);
      buffer[6] = 'P', buffer[7] = 'M';
      buffer[8] = 'C', buffer[9] = 'C';
      if ((UINT)HFILE_ERROR == _lwrite(file, buffer, 10))
              return FALSE;

      seqnum = 0;
      hProgram = group->hPrograms;
      while(hProgram)
        {
          PROGRAM *program = (PROGRAM *)LocalLock(hProgram);
          LPCSTR lpszWorkDir = LocalLock(program->hWorkDir);

          /* Working directory */
          if (lpszWorkDir[0])
            {
              PUT_SHORT(buffer, 0, 0x8101);
              PUT_SHORT(buffer, 2, seqnum);
              PUT_SHORT(buffer, 4, 7 + lstrlen(lpszWorkDir));
              if ((UINT)HFILE_ERROR == _lwrite(file, buffer, 6) ||
                  (UINT)HFILE_ERROR == _lwrite(file, lpszWorkDir, lstrlen(lpszWorkDir) + 1))
                return FALSE;
            }

          /* Hot key */
          if (program->nHotKey)
            {
              PUT_SHORT(buffer, 0, 0x8102);
              PUT_SHORT(buffer, 2, seqnum);
              PUT_SHORT(buffer, 4, 8);
              PUT_SHORT(buffer, 6, program->nHotKey);
              if ((UINT)HFILE_ERROR == _lwrite(file, buffer, 8)) return FALSE;
            }

          /* Show command */
          if (program->nCmdShow)
            {
              PUT_SHORT(buffer, 0, 0x8103);
              PUT_SHORT(buffer, 2, seqnum);
              PUT_SHORT(buffer, 4, 8);
              PUT_SHORT(buffer, 6, program->nCmdShow);
              if ((UINT)HFILE_ERROR == _lwrite(file, buffer, 8)) return FALSE;
            }

          seqnum++;
          hProgram = program->hNext;
        }

      /* Write `End' extension */
      PUT_SHORT(buffer, 0, 0xffff);
      PUT_SHORT(buffer, 2, 0xffff);
      PUT_SHORT(buffer, 4, 0x0000);
      if ((UINT)HFILE_ERROR == _lwrite(file, buffer, 6)) return FALSE;
    }

  checksum = GRPFILE_GetChecksum();
  _llseek(file, 4, SEEK_SET);
  PUT_SHORT(buffer, 0, checksum);
  _lwrite(file, buffer, 2);

  return TRUE;
}

/* Local Variables:    */
/* c-file-style: "GNU" */
/* End:                */
