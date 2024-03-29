/*!

   @file movefile.c

   @brief movefile API implementation. Based on DosMove from OS/2 Personality..

   (c) osFree Project 2008, 2023, <http://www.osFree.org>
   for licence see licence.txt in root directory, or project website

   @author Yuri Prokushev <yuri.prokushev@gmail.com>

*/

#include <string.h>
#include <sys/stat.h>

//#include "kal.h"

// Safe functions
//#include "strnlen.h"
//#include "strlcpy.h"
//#include "strlcat.h"
//#include "strlcmp.h"

#include <windows.h>

/*!
   @brief Copies/moves file trees from one location to another

   @param pszSrc             name of source file
   @param pszDst             name of destination (may be existent)
   @param ulOptions          copy options

   @return
     NO_ERROR           - files were processed succesfully
     ERROR_WRITE_FAULT  - fault during file writing
     other              - according errors in used APIs

   API
     DosAllocMem
     DosFreeMem
     DosOpenL
     DosClose
     DosRead
     DosWrite
     DosSetFilePtrL
     DosCreateDir
     DosDeleteDir
     DosDelete
     DosFindFirst
     DosFindNext
     DosFindClose
     DosQueryPathInfo
     KalMove
*/

//APIRET __cdecl   KalMove(PCSZ  pszOld,
//                         PCSZ  pszNew);

#define IOBUF_SIZ       32768U                  /* enough? (performance)
                                                   Most probably here
                                                   we must automatically
                                                   detect size of buffer */

APIRET CopyFile(PSZ pszSrc, PSZ pszDst, ULONG ulOptions)
{
  APIRET rc;
  HFILE  hSrc;
  HFILE  hDst;
  ULONG  ulAction;
  //PCHAR
  void **pIOBuf;
  ULONG  ulTransfer;
  ULONG  ulWritten;
  ULONG  ulOpenType;
  LONGLONG llZero;

  llZero.ulLo=0;
  llZero.ulHi=0;

  rc = DosAllocMem(pIOBuf,
                   IOBUF_SIZ,
                   fPERM|PAG_COMMIT);
  if (rc) return rc;

  rc = DosOpenL(pszSrc,             // Address of ASCIIZ with source path
                &hSrc,              // Handle
                &ulAction,          // Action was taken (not used)
                llZero,             // Initial file size (not used)
                0,                  // File attributes (not used)
                OPEN_ACTION_FAIL_IF_NEW |
                OPEN_ACTION_OPEN_IF_EXISTS, // Open type
                OPEN_SHARE_DENYNONE |
                OPEN_ACCESS_READONLY, // Open mode
                NULL);
  if (rc)
  {
    DosClose(hSrc);
    DosFreeMem(pIOBuf);
    return rc;
  }

  if (!(ulOptions&DCPY_EXISTING))
  {
    // if no file exists then we fail
    ulOpenType = OPEN_ACTION_FAIL_IF_EXISTS;
  } else {
    if (ulOptions&DCPY_APPEND)
    {
      // else or append
      ulOpenType = OPEN_ACTION_OPEN_IF_EXISTS;
    } else {
      // or replace
      ulOpenType = OPEN_ACTION_REPLACE_IF_EXISTS;
    }
  }

  rc = DosOpenL(pszDst,             // Address of ASCIIZ with source path
                &hDst,              // Handle
                &ulAction,          // Action was taken
                llZero,             // Initial file size (not used)
                FILE_ARCHIVED |
                FILE_NORMAL, // File attributes
                OPEN_ACTION_CREATE_IF_NEW |
                ulOpenType, // Open type
                OPEN_SHARE_DENYREADWRITE |
                OPEN_ACCESS_WRITEONLY, // Open mode
                NULL);
  if (rc)
  {
    DosClose(hDst);
    DosClose(hSrc);
    DosFreeMem(pIOBuf);
    return rc;
  }


  // If append mode move file pointer to the end
  // We can be here only if not DCY_EXISTING flag
  // set because DosOpenL will fail
  // In case of APPEND we need to move to end of file (because file open
  // with OPEN_ACCESS_OPEN_IF_EXISTS) otherwise
  if (ulOptions&DCPY_APPEND)
  {
    DosSetFilePtrL (hDst,
                    llZero,
                    FILE_END,
                    NULL);
  }

  rc = DosRead(hSrc, pIOBuf, IOBUF_SIZ, &ulTransfer);
  if (rc)
  {
    DosClose(hDst);
    DosClose(hSrc);
    DosFreeMem(pIOBuf);
    return rc;
  }

  while(ulTransfer)
  {
    rc = DosWrite(hDst, pIOBuf, ulTransfer, &ulWritten);
    if (rc)
    {
      DosClose(hDst);
      DosClose(hSrc);
      DosFreeMem(pIOBuf);
      return rc;
    }

    if (ulTransfer!=ulWritten) return ERROR_WRITE_FAULT;

    rc = DosRead(hSrc, pIOBuf, IOBUF_SIZ, &ulTransfer);
    if (rc)
    {
      DosClose(hDst);
      DosClose(hSrc);
      DosFreeMem(pIOBuf);
      return rc;
    }
  }

  DosClose(hDst);
  DosClose(hSrc);
  DosFreeMem(pIOBuf);
  return NO_ERROR;
}

/*#
 * NAME
 *      CheckPath
 * CALL
 *      CheckPath(path,create)
 * PARAMETER
 *      path            absolute directory name
 *      create          create directory if not existing
 * RETURNS
 *      0               directory is now existing
 *      /0              file, not existing, etc.
 * GLOBAL
 *      none
 * DESPRIPTION
 * REMARKS
 */
int
CheckPath(char *path,int create)
{
    char   dir[CCHMAXPATH];
    struct stat stbuf;

    strlcpy( dir, path, CCHMAXPATH );
    if( dir[strnlen(dir, CCHMAXPATH)-1] == '/'  &&  dir[strnlen(dir, CCHMAXPATH)-2] != ':' )
        dir[strnlen(dir, CCHMAXPATH)-1] = '\0';
        /* TODO!!!!!! We need to check is directory exists here. Most probably via dosfindfirst...
    if( stat(dir, &stbuf) != 0 )
    {
        if( !create )
        {
//            Verbose(1,"stat(%s) - errno %u (%s)", dir, errno, strerror(errno) );
            return 0;//errno;
        }
        else
        {
            if( DosCreateDir(dir, NULL))
            {
//                Verbose(1,"mkdir(%s) - errno %u (%s)",dir,errno,strerror(errno));
                return 0;//errno;
            }
        }
    }
    else
    {
        if( (stbuf.st_mode & S_IFMT) != S_IFDIR )
        {
//            Verbose(1,"stat(%s) - no directory",dir);
            return -1;
        }
    }
*/
    return 0;
}


/*!
    @brief Copy directory tree

    @param src             Source Path (existing)
    @param dst             Destination Path (existing)
    @param ulOption        copy options

    @return
        NO_ERROR               OK

 */
APIRET CopyTree(PSZ pszSrc, PSZ pszDst, ULONG ulOptions, ULONG ulNeedDel)
{
    FILEFINDBUF3  findBuffer;
    HDIR         hSearch;
    ULONG        cFound;
    APIRET       rc;
    int          result = 0, i;
    char        *nsp, *ndp;
    struct dirlist_t {
        char              src[CCHMAXPATH]; //@todo detect maximum path name!!!
        char              dst[CCHMAXPATH]; //@todo detect maximum path name!!!
        struct dirlist_t *next;
    } *pDirListRoot=NULL, *pDirList=NULL, *pHelp;

    nsp = pszSrc + strnlen(pszSrc, CCHMAXPATH);
    ndp = pszDst + strnlen(pszDst, CCHMAXPATH);

    /* Search all subdirectories */

    strlcpy( nsp, "*", CCHMAXPATH );
    hSearch = HDIR_SYSTEM;                      /* use system handle */
    cFound = 1;                                 /* only one at a time */
    rc = DosFindFirst(pszSrc,
                      &hSearch,
                      MUST_HAVE_DIRECTORY|FILE_DIRECTORY,
                      &findBuffer,
                      sizeof(findBuffer),
                      &cFound,
                      FIL_STANDARD);
    if( !rc )
        do
        {
            if( !strncmp(findBuffer.achName, ".", CCHMAXPATH)
               ||  !strncmp(findBuffer.achName, "..", CCHMAXPATH) )
                continue;
            if( !(findBuffer.attrFile & FILE_DIRECTORY) )
                continue;

            strlcpy( nsp, findBuffer.achName, CCHMAXPATH );
            strlcpy( ndp, findBuffer.achName, CCHMAXPATH );

            strlcat( pszSrc, "/", CCHMAXPATH );
            strlcat( pszDst, "/", CCHMAXPATH );


            if( pDirList )
            {
                rc = DosAllocMem((void **)&(pDirList->next),
                                 sizeof(struct dirlist_t),
                                 fPERM|PAG_COMMIT);
//                assert( pDirList->next != NULL );
                pDirList = pDirList->next;
            }
            else
            {
                rc = DosAllocMem((void **)&(pDirListRoot),
                                 sizeof(struct dirlist_t),
                                 fPERM|PAG_COMMIT);
                pDirList = pDirListRoot;
//                assert( pDirList != NULL );
            }
            pDirList->next = NULL;
            strlcpy( pDirList->src, pszSrc, CCHMAXPATH );
            strlcpy( pDirList->dst, pszDst, CCHMAXPATH );
        }
        while( !(rc=DosFindNext(hSearch, &findBuffer,
                                sizeof(findBuffer), &cFound)) );
    DosFindClose( hSearch );

    for( pHelp = pDirList = pDirListRoot; pDirList ; pHelp = pDirList )
    {
        pDirList = pDirList->next;
        if( (i=CheckPath(pHelp->dst, 1)) )      /* create destination path */
        {
            result = i;
        }
        else
        {
            CopyTree( pHelp->src, pHelp->dst, ulOptions, ulNeedDel );
        }
        DosFreeMem(pHelp);
    }

    /* Copy the files in actual directory */

    strlcpy( nsp, "*", CCHMAXPATH );
    hSearch = HDIR_SYSTEM;                      /* use system handle */
    cFound = 1;                                 /* only one at a time */
    rc = DosFindFirst(pszSrc, &hSearch, FILE_NORMAL,
                      &findBuffer, sizeof(findBuffer), &cFound, FIL_STANDARD );
    if( !rc )
        do
        {
            if( findBuffer.attrFile & FILE_DIRECTORY )
                continue;

            strlcpy( nsp, findBuffer.achName, CCHMAXPATH );
            strlcpy( ndp, findBuffer.achName, CCHMAXPATH );
            i = CopyFile( pszSrc, pszDst, ulOptions );

            // Delete original file,
            // if needed
            if (ulNeedDel)
              DosDelete(pszSrc);

            if( i != 0 )
                result = i;
        }
        while( !(rc=DosFindNext(hSearch, &findBuffer,
                                sizeof(findBuffer), &cFound)) );
    DosFindClose( hSearch );

    // Delete original dir,
    // if needed
    if (ulNeedDel)
      DosDeleteDir(pszSrc);

    *nsp = '\0';
    *ndp = '\0';
    return result;
}

BOOL WINAPI DosMove(LPCTSTR pszOld, LPCTSTR pszNew)
{
  FILESTATUS3 fileStatus;
  APIRET rc;

  log("%s enter\n", __FUNCTION__);
  log("pszOld=%s\n", pszOld);
  log("pszNew=%s\n", pszNew);

  //Check arguments
  if ((!pszOld) || (!pszNew)) return ERROR_INVALID_PARAMETER;

  // if move pszOld->pszNew crosses the volume
  // boundary, then copy files and delete the old ones
  if (KalMove((PSZ)pszOld, (PSZ)pszNew))
  {
    //Detect is source dir or file (also check is it exists)
    rc = DosQueryPathInfo(pszOld,               // Path
                          FIL_STANDARD,         // Level 1 information
                          &fileStatus,          // Address of return buffer
                          sizeof(FILESTATUS3)); // Size of buffer

    if (rc)
    {
      goto DOSMOVE_EXIT;
    }

    // Perfom action based on source path type
    if (fileStatus.attrFile & FILE_DIRECTORY)
      // DCPY_APPEND flag not valid in directory copy
      rc = CopyTree((PSZ)pszOld, (PSZ)pszNew, 0, 1);
    else
    {
      if (rc = CopyFile((PSZ)pszOld, (PSZ)pszNew, 0))
      {
        goto DOSMOVE_EXIT;
      }

      rc = DosDelete(pszOld);
    }
  }

DOSMOVE_EXIT:
  log("%s exit => %lx\n", __FUNCTION__, rc);
  return rc;
}
