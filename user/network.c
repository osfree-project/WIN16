/*
 * USER Windows Network functions
 *
 * Copyright 1995 Martin von Loewis
 *
This library is free software; you can redistribute it and/or
modify it under the terms of the GNU Lesser General Public
License as published by the Free Software Foundation; either
version 2.1 of the License, or (at your option) any later version.

This library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public
License along with this library; if not, see
<https://www.gnu.org/licenses/>.

 */

#include <ctype.h>
#include <stdarg.h>
#include <string.h>
#include <sys/types.h>

#include <windows.h>
//#include "winbase.h"
//#include "winnetwk.h"

#include "winnet.h"

//#include "wine/debug.h"

//WINE_DEFAULT_DEBUG_CHANNEL(wnet);

/*
 * Remote printing
 */

/**************************************************************************
 *              WNetOpenJob       [USER.501]
 */
WORD WINAPI WNetOpenJob( LPSTR szQueue, LPSTR szJobTitle, WORD nCopies, LPINT pfh )
{
//    FIXME( "(%s, %s, %d, %p): stub\n",
//           debugstr_a(szQueue), debugstr_a(szJobTitle), nCopies, pfh );
    return WN16_NET_ERROR;
}

/**************************************************************************
 *              WNetCloseJob      [USER.502]
 */
WORD WINAPI WNetCloseJob( WORD fh, LPINT pidJob, LPSTR szQueue )
{
//    FIXME( "(%d, %p, %s): stub\n", fh, pidJob, debugstr_a(szQueue) );
    return WN16_NET_ERROR;
}

/**************************************************************************
 *              WNetWriteJob      [USER.524]
 */
WORD WINAPI WNetWriteJob( HANDLE hJob, LPSTR lpData, LPINT lpcbData )
{
//    FIXME( "(%04x, %p, %p): stub\n", hJob, lpData, lpcbData );
    return WN16_NET_ERROR;
}

/**************************************************************************
 *              WNetAbortJob       [USER.503]
 */
WORD WINAPI WNetAbortJob( LPSTR szQueue, WORD wJobId )
{
//    FIXME( "(%s, %d): stub\n", debugstr_a(szQueue), wJobId );
    return WN16_NET_ERROR;
}

/**************************************************************************
 *              WNetHoldJob       [USER.504]
 */
WORD WINAPI WNetHoldJob( LPSTR szQueue, WORD wJobId )
{
//    FIXME( "(%s, %d): stub\n", debugstr_a(szQueue), wJobId );
    return WN16_NET_ERROR;
}

/**************************************************************************
 *              WNetReleaseJob       [USER.505]
 */
WORD WINAPI WNetReleaseJob( LPSTR szQueue, WORD wJobId )
{
//    FIXME( "(%s, %d): stub\n", debugstr_a(szQueue), wJobId );
    return WN16_NET_ERROR;
}

/**************************************************************************
 *              WNetCancelJob       [USER.506]
 */
WORD WINAPI WNetCancelJob( LPSTR szQueue, WORD wJobId )
{
//    FIXME( "(%s, %d): stub\n", debugstr_a(szQueue), wJobId );
    return WN16_NET_ERROR;
}

/**************************************************************************
 *              WNetSetJobCopies     [USER.507]
 */
WORD WINAPI WNetSetJobCopies( LPSTR szQueue, WORD wJobId, WORD nCopies )
{
//    FIXME( "(%s, %d, %d): stub\n", debugstr_a(szQueue), wJobId, nCopies );
    return WN16_NET_ERROR;
}

/**************************************************************************
 *              WNetWatchQueue       [USER.508]
 */
WORD WINAPI WNetWatchQueue( HWND hWnd, LPSTR szLocal, LPSTR szUser, WORD nQueue )
{
//    FIXME( "(%04x, %s, %s, %d): stub\n",
//           hWnd, debugstr_a(szLocal), debugstr_a(szUser), nQueue );
    return WN16_NET_ERROR;
}

/**************************************************************************
 *              WNetUnwatchQueue     [USER.509]
 */
WORD WINAPI WNetUnwatchQueue( LPSTR szQueue )
{
//    FIXME( "(%s): stub\n", debugstr_a(szQueue) );
    return WN16_NET_ERROR;
}

/**************************************************************************
 *              WNetLockQueueData       [USER.510]
 */
WORD WINAPI WNetLockQueueData( LPSTR szQueue, LPSTR szUser,
                                 LPQUEUESTRUCT far *lplpQueueStruct )
{
//    FIXME( "(%s, %s, %p): stub\n",
//           debugstr_a(szQueue), debugstr_a(szUser), lplpQueueStruct );
    return WN16_NET_ERROR;
}

/**************************************************************************
 *              WNetUnlockQueueData       [USER.511]
 */
WORD WINAPI WNetUnlockQueueData( LPSTR szQueue )
{
//    FIXME( "(%s): stub\n", debugstr_a(szQueue) );
    return WN16_NET_ERROR;
}


/*
 * Connections
 */

/********************************************************************
 *  WNetAddConnection [USER.517]  Directs a local device to net
 *
 * Redirects a local device (either a disk drive or printer port)
 * to a shared device on a remote server.
 */
UINT WINAPI WNetAddConnection( LPSTR lpNetPath, LPSTR lpPassWord,
                                 LPSTR lpLocalName )
{
//    FIXME( "(%s, %p, %s): stub\n",
//           debugstr_a(lpNetPath), lpPassWord, debugstr_a(lpLocalName) );
    return WN16_NET_ERROR;
}

/********************************************************************
 *   WNetCancelConnection [USER.518]  undirects a local device
 */
UINT WINAPI WNetCancelConnection( LPSTR lpName, BOOL bForce )
{
//    FIXME( "(%s, %04X): stub\n", debugstr_a(lpName), bForce);
    return WN16_NOT_SUPPORTED;
}

/********************************************************************
 * WNetGetConnection [USER.512] reverse-resolves a local device
 */
/*
UINT WINAPI WNetGetConnection( LPSTR lpLocalName,
                                 LPSTR lpRemoteName, UINT far *cbRemoteName )
{
    char label[32];

//    TRACE( "local %s\n", lpLocalName );
    switch(GetDriveType(lpLocalName))
    {
    case DRIVE_REMOTE:
        GetVolumeInformation( lpLocalName, label, sizeof(label), NULL, NULL, NULL, NULL, 0 );
        if (lstrlen(label) + 1 > *cbRemoteName)
        {
            *cbRemoteName = lstrlen(label) + 1;
            return WN16_MORE_DATA;
        }
        lstrcpy( lpRemoteName, label );
        *cbRemoteName = lstrlen(lpRemoteName) + 1;
        return WN16_SUCCESS;
    case DRIVE_REMOVABLE:
    case DRIVE_FIXED:
    case DRIVE_CDROM:
//        TRACE("file is local\n");
        return WN16_NOT_CONNECTED;
    default:
        return WN16_BAD_LOCALNAME;
    }
}
*/
/**************************************************************************
 *              WNetRestoreConnection       [USER.523]
 */
WORD WINAPI WNetRestoreConnection( HWND hwndOwner, LPSTR lpszDevice )
{
//    FIXME( "(%04x, %s): stub\n", hwndOwner, debugstr_a(lpszDevice) );
    return WN16_NOT_SUPPORTED;
}


/*
 * Capabilities
 */

/**************************************************************************
 *		WNetGetCaps		[USER.513]
 */
WORD WINAPI WNetGetCaps( WORD capability )
{
    switch (capability)
    {
    case WNNC16_SPEC_VERSION:
        return 0x30a; /* WfW 3.11 (and apparently other 3.1x) */

    case WNNC16_NET_TYPE:
        /* hi byte = network type,
           lo byte = network vendor (Netware = 0x03) [15 types] */
        return WNNC16_NET_MultiNet | WNNC16_SUBNET_WinWorkgroups;

    case WNNC16_DRIVER_VERSION:
        /* driver version of vendor */
        return 0x100; /* WfW 3.11 */

    case WNNC16_USER:
        /* 1 = WNetGetUser is supported */
        return 1;

    case WNNC16_CONNECTION:
        /* returns mask of the supported connection functions */
        return   WNNC16_CON_AddConnection | WNNC16_CON_CancelConnection
               | WNNC16_CON_GetConnections /* | WNNC16_CON_AutoConnect */
               | WNNC16_CON_BrowseDialog | WNNC16_CON_RestoreConnection;

    case WNNC16_PRINTING:
        /* returns mask of the supported printing functions */
        return   WNNC16_PRT_OpenJob | WNNC16_PRT_CloseJob | WNNC16_PRT_HoldJob
               | WNNC16_PRT_ReleaseJob | WNNC16_PRT_CancelJob
               | WNNC16_PRT_SetJobCopies | WNNC16_PRT_WatchQueue
               | WNNC16_PRT_UnwatchQueue | WNNC16_PRT_LockQueueData
               | WNNC16_PRT_UnlockQueueData | WNNC16_PRT_AbortJob
               | WNNC16_PRT_WriteJob;

    case WNNC16_DIALOG:
        /* returns mask of the supported dialog functions */
        return   WNNC16_DLG_DeviceMode | WNNC16_DLG_BrowseDialog
               | WNNC16_DLG_ConnectDialog | WNNC16_DLG_DisconnectDialog
               | WNNC16_DLG_ViewQueueDialog | WNNC16_DLG_PropertyDialog
               | WNNC16_DLG_ConnectionDialog
            /* | WNNC16_DLG_PrinterConnectDialog
               | WNNC16_DLG_SharesDialog | WNNC16_DLG_ShareAsDialog */;

    case WNNC16_ADMIN:
        /* returns mask of the supported administration functions */
        /* not sure if long file names is a good idea */
        return   WNNC16_ADM_GetDirectoryType
            /* | WNNC16_ADM_DirectoryNotify */ /*not yet supported*/
               | WNNC16_ADM_LongNames /* | WNNC16_ADM_SetDefaultDrive */;

    case WNNC16_ERROR:
        /* returns mask of the supported error functions */
        return   WNNC16_ERR_GetError | WNNC16_ERR_GetErrorText;

    case WNNC16_PRINTMGREXT:
        /* returns the Print Manager version in major and
           minor format if Print Manager functions are available */
        return 0x30e; /* printman version of WfW 3.11 */

    case 0xffff:
        /* Win 3.11 returns HMODULE of network driver here
           FIXME: what should we return ?
           logonoff.exe needs it, msmail crashes with wrong value */
        return 0;

    default:
        return 0;
    }
}


/*
 * Browsing
 */

/**************************************************************************
 *              WNetDeviceMode       [USER.514]
 */
WORD WINAPI WNetDeviceMode( HWND hWndOwner )
{
//    FIXME( "(%04x): stub\n", hWndOwner );
    return WN16_NOT_SUPPORTED;
}

/**************************************************************************
 *              WNetBrowseDialog       [USER.515]
 */
WORD WINAPI WNetBrowseDialog( HWND hParent, WORD nType, LPSTR szPath )
{
//    FIXME( "(%04x, %x, %s): stub\n", hParent, nType, szPath );
    return WN16_NOT_SUPPORTED;
}

/********************************************************************
 *              WNetConnectDialog       [USER.525]
 */
WORD WINAPI WNetConnectDialog( HWND hWndParent, WORD iType )
{
//    FIXME( "(%04x, %x): stub\n", hWndParent, iType );
    return WN16_SUCCESS;
}

/**************************************************************************
 *              WNetDisconnectDialog       [USER.526]
 */
WORD WINAPI WNetDisconnectDialog( HWND hwndOwner, WORD iType )
{
//    FIXME( "(%04x, %x): stub\n", hwndOwner, iType );
    return WN16_NOT_SUPPORTED;
}

/**************************************************************************
 *              WNetConnectionDialog     [USER.527]
 */
WORD WINAPI WNetConnectionDialog( HWND hWndParent, WORD iType )
{
//    FIXME( "(%04x, %x): stub\n", hWndParent, iType );
    return WN16_SUCCESS;
}

/**************************************************************************
 *              WNetViewQueueDialog       [USER.528]
 */
WORD WINAPI WNetViewQueueDialog( HWND hwndOwner, LPSTR lpszQueue )
{
//    FIXME(" (%04x, %s): stub\n", hwndOwner, debugstr_a(lpszQueue) );
    return WN16_NOT_SUPPORTED;
}

/**************************************************************************
 *              WNetPropertyDialog       [USER.529]
 */
WORD WINAPI WNetPropertyDialog( HWND hwndParent, WORD iButton,
                                  WORD nPropSel, LPSTR lpszName, WORD nType )
{
//    FIXME( "(%04x, %x, %x, %s, %x ): stub\n",
//          hwndParent, iButton, nPropSel, debugstr_a(lpszName), nType );
    return WN16_NOT_SUPPORTED;
}

/**************************************************************************
 *              WNetGetPropertyText       [USER.532]
 */
WORD WINAPI WNetGetPropertyText( WORD iButton, WORD nPropSel, LPSTR lpszName,
                                   LPSTR lpszButtonName, WORD cbButtonName, WORD nType )
{
//    FIXME( "(%04x, %04x, %s, %s, %04x): stub\n",
//           iButton, nPropSel, debugstr_a(lpszName), debugstr_a(lpszButtonName), nType);
    return WN16_NOT_SUPPORTED;
}


/*
 * Admin
 */

/*********************************************************************
 *  WNetGetDirectoryType [USER.530]  Decides whether resource is local
 *
 * RETURNS
 *    on success,  puts one of the following in *lpType:
 * - WNDT_NETWORK   on a network
 * - WNDT_LOCAL     local
 */
/*
WORD WINAPI WNetGetDirectoryType( LPSTR lpName, LPINT lpType )
{
    UINT type = GetDriveType(lpName);
    if ( type == DRIVE_NO_ROOT_DIR )
        type = GetDriveType(NULL);

    *lpType = (type == DRIVE_REMOTE)? WNDT_NETWORK : WNDT_NORMAL;

//    TRACE( "%s is %s\n", debugstr_a(lpName),
//           (*lpType == WNDT_NETWORK)? "WNDT_NETWORK" : "WNDT_NORMAL" );
    return WN16_SUCCESS;
}
*/
/**************************************************************************
 *              WNetDirectoryNotify       [USER.531]
 */
WORD WINAPI WNetDirectoryNotify( HWND hwndOwner, LPSTR lpDir, WORD wOper )
{
//    FIXME( "(%04x, %s, %s): stub\n", hwndOwner, debugstr_a(lpDir),
//           (wOper == WNDN_MKDIR)? "WNDN_MKDIR" :
//           (wOper == WNDN_MVDIR)? "WNDN_MVDIR" :
//           (wOper == WNDN_RMDIR)? "WNDN_RMDIR" : "unknown" );
    return WN16_NOT_SUPPORTED;
}


/*
 * Error handling
 */

/**************************************************************************
 *              WNetGetError       [USER.519]
 */
WORD WINAPI WNetGetError( LPINT nError )
{
//    FIXME( "(%p): stub\n", nError );
    return WN16_NOT_SUPPORTED;
}

/**************************************************************************
 *              WNetGetErrorText       [USER.520]
 */
WORD WINAPI WNetGetErrorText( WORD nError, LPSTR lpBuffer, LPINT nBufferSize )
{
//    FIXME( "(%x, %p, %p): stub\n", nError, lpBuffer, nBufferSize );
    return WN16_NET_ERROR;
}

/**************************************************************************
 *              WNetErrorText       [USER.499]
 */
WORD WINAPI WNetErrorText( WORD nError, LPSTR lpszText, WORD cbText )
{
//    FIXME("(%x, %p, %x): stub\n", nError, lpszText, cbText );
    return FALSE;
}
