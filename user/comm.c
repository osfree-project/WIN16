
#include <windows.h>

/**************************************************************************
 *         BuildCommDCB		(USER.213)
 *
 * According to the ECMA-234 (368.3) the function will return FALSE on
 * success, otherwise it will return -1.
 */
int WINAPI BuildCommDCB(LPCSTR device, LPDCB lpdcb)
{
	return 0;
}

/*****************************************************************************
 *	OpenComm		(USER.200)
 */
int WINAPI OpenComm(LPCSTR device,UINT cbInQueue,UINT cbOutQueue)
{
	return 0;
}

/*****************************************************************************
 *	CloseComm		(USER.207)
 */
int WINAPI CloseComm(int cid)
{
		return 0;
}

/*****************************************************************************
 *	SetCommBreak		(USER.210)
 */
int WINAPI SetCommBreak(int cid)
{
	return 0;
}

/*****************************************************************************
 *	ClearCommBreak	(USER.211)
 */
int WINAPI ClearCommBreak(int cid)
{
	return 0;
}

/*****************************************************************************
 *	EscapeCommFunction	(USER.214)
 */
LONG WINAPI EscapeCommFunction(int cid,int nFunction)
{
	return -1;
}

/*****************************************************************************
 *	FlushComm	(USER.215)
 */
int WINAPI FlushComm(int cid,int fnQueue)
{
		return 0;
}

/********************************************************************
 *	GetCommError	(USER.203)
 */
int WINAPI GetCommError(int cid, COMSTAT far * lpStat)
{
	return(0);
}

/*****************************************************************************
 *	SetCommEventMask	(USER.208)
 */
UINT far * WINAPI SetCommEventMask(int cid,UINT fuEvtMask)
{
        return 0;
}

/*****************************************************************************
 *	GetCommEventMask	(USER.209)
 */
UINT WINAPI GetCommEventMask(int cid,int fnEvtClear)
{
	return 0;
}

/*****************************************************************************
 *	SetCommState	(USER.201)
 */
int WINAPI SetCommState(const DCB far * lpdcb)
{
		return 0;
}

/*****************************************************************************
 *	GetCommState	(USER.202)
 */
int WINAPI GetCommState16(int cid, LPDCB lpdcb)
{
	return 0;
}

/*****************************************************************************
 *	TransmitCommChar	(USER.206)
 */
int WINAPI TransmitCommChar(int cid,char chTransmit)
{
	return 0;
}

/*****************************************************************************
 *	UngetCommChar	(USER.212)
 */
int WINAPI UngetCommChar(int cid,char chUnget)
{
	return 0;
}

/*****************************************************************************
 *	ReadComm	(USER.204)
 */
int WINAPI ReadComm(int cid, void far * lpvBuf,int cbRead)
{
	return 0;
}

/*****************************************************************************
 *	WriteComm	(USER.205)
 */
int WINAPI WriteComm(int cid, void const far * lpvBuf, int cbWrite)
{
	return 0;
}

/***********************************************************************
 *           EnableCommNotification   (USER.245)
 */
BOOL WINAPI EnableCommNotification( int cid, HWND hwnd,
                                      int cbWriteNotify, int cbOutQueue )
{
	return FALSE;
}
