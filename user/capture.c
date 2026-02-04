#include <user.h>

/**********************************************************************
 *		ReleaseCapture	(USER.19)
 */
void WINAPI ReleaseCapture()
{
	PushDS();
	SetUserHeapDS();

	captureWnd = 0;

	PopDS();
}

/**********************************************************************
 *		SetCapture 	(USER.18)
 */
HWND WINAPI SetCapture( HWND hWnd )
{
	HWND retVal;

	PushDS();
	SetUserHeapDS();

	retVal = captureWnd;
	captureWnd = hWnd;

	PopDS();
	return retVal;
}

/**********************************************************************
 *		GetCapture 	(USER.236)
 */
HWND WINAPI GetCapture()
{
	HWND retVal;

	PushDS();
	SetUserHeapDS();

	retVal=captureWnd;

	PopDS();

	return retVal;
}
