#include <user.h>

/**********************************************************************
 *		SetCapture 	(USER.18)
 */
HWND WINAPI SetCapture( HWND hWnd )
{
	HWND old_capture_wnd;

	PushDS();
	SetUserHeapDS();

	old_capture_wnd = captureWnd;

	if (!hWnd)
	{
		ReleaseCapture();

		PopDS();
		return old_capture_wnd;
	}

	captureWnd   = hWnd;

	PopDS();
	return old_capture_wnd;
}


/**********************************************************************
 *		ReleaseCapture	(USER.19)
 */
void WINAPI ReleaseCapture()
{
	PushDS();
	SetUserHeapDS();

	if (captureWnd == 0)
	{
		PopDS();
		return;
	}
	captureWnd = 0;

	PopDS();
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
