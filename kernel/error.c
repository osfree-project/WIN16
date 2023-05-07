#include <windows.h>

static DWORD dwLastError = 0;

DWORD	WINAPI
GetLastError(VOID)
{
//	DWORD rc;
//	APISTR((LF_APICALL, "GetLastError()\n"));

//	rc = dwLastError;

//	APISTR((LF_APIRET, "GetLastError: returns DWORD %x\n",rc));

	return dwLastError;
}

VOID	WINAPI
SetLastError(DWORD dwError)
{
//	APISTR((LF_APICALL, "SetLastError(DWORD=%x)\n",dwError));
	dwLastError = dwError;
//	APISTR((LF_APIRET, "SetLastError: returns void\n"));
}
