#include <user.h>

/**********************************************************************
 *              LoadAccelerators  (USER.177)
 */
HACCEL	WINAPI
LoadAccelerators(HINSTANCE hInstance, LPCSTR lpTableName)
{
	HANDLE hResInfo;
	HACCEL rc;

	FUNCTION_START
//    	APISTR((LF_APICALL,"LoadAccelerators(HINSTANCE=%x,LPCSTR=%x)\n",
//		hInstance,lpTableName));

	hResInfo = FindResource(hInstance,lpTableName,RT_ACCELERATOR);

	if(hResInfo == 0) {
//    		APISTR((LF_APIFAIL,"LoadAccelerators: returns HACCEL 0\n"));
		return 0;
	}

	rc =  LoadResource(hInstance,hResInfo);
//    	APISTR((LF_APIRET,"LoadAccelerators: returns HACCEL %x\n",rc));
	return rc;
}
