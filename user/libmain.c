#include <user.h>

extern  unsigned short          GetDS( void );
#pragma aux GetDS               = \
        "mov    ax,ds"          \
        value                   [ax];

int UT_GetIntFromProfile(UINT id, int defvalue)
{
	char section[30];
	char key[30];
	char value[30];

	FUNCTION_START

	LoadString(USER_HeapSel, IDS_WINDOWS, section, sizeof(section));
	LoadString(USER_HeapSel, id, key, sizeof(key));

	// Protect from empty value
	if (!GetProfileString(section, key, "", value, sizeof(value)))
	{
		FUNCTION_END
		return defvalue;
	}

	FUNCTION_END
	return GetProfileInt(section, key, defvalue);
}

VOID CreateQueue(int QueueSize)
{
	LPQUEUE lpQueue;
	int size;

	FUNCTION_START

	size=sizeof(QUEUE)+(QueueSize-1)*sizeof(INTERNALMSG);

	lpQueue=(LPQUEUE)GlobalAlloc(GPTR+GMEM_SHARE, size);
	lpQueue->OwningTask=GetCurrentTask();
	lpQueue->MessageSize=sizeof(INTERNALMSG);
	lpQueue->Size=QueueSize;
	//lpQueue->ReadPtr=???;
	//lpQueue->WritePtr=???;
	//lpQueue->ExpWinVersion=GetExeVersion();
	//lpQueue->WakeBits=QS_SMPARAMSFREE;
	//lpQueue->flags=QF_INIT;

	SetTaskQueue(0, (HGLOBAL)lpQueue);

	FUNCTION_END
}

VOID WINAPI LW_LoadSomeStrings()
{
	FUNCTION_START
	LoadString(USER_HeapSel, IDS_SYSTEMERROR, szSysError, sizeof(szSysError));
	LoadString(USER_HeapSel, IDS_DIVIDEBYZERO, szDivZero, sizeof(szDivZero));
	LoadString(USER_HeapSel, IDS_UNTITLED, szUntitled, sizeof(szUntitled));
	LoadString(USER_HeapSel, IDS_ERROR, szError, sizeof(szError));
	LoadString(USER_HeapSel, IDS_OK, szOk, sizeof(szOk));
	LoadString(USER_HeapSel, IDS_CANCEL, szCancel, sizeof(szCancel));
	LoadString(USER_HeapSel, IDS_ABORT, szAbort, sizeof(szAbort));
	LoadString(USER_HeapSel, IDS_RETRY, szRetry, sizeof(szRetry));
	LoadString(USER_HeapSel, IDS_IGNORE, szIgnore, sizeof(szIgnore));
	LoadString(USER_HeapSel, IDS_YES, szYes, sizeof(szYes));
	LoadString(USER_HeapSel, IDS_NO, szNo, sizeof(szNo));
	// LoadString(USER_HeapSel, 0x4b, szClose, sizeof(szClose)); not found in user.exe resources
	LoadString(USER_HeapSel, IDS_AM, szAm, sizeof(szAm));
	LoadString(USER_HeapSel, IDS_PM, szPm, sizeof(szPm));
	FUNCTION_END
}

VOID WINAPI RW_RegisterMenus()
{
	FUNCTION_START

	FUNCTION_END
}

VOID WINAPI RW_RegisterButton()
{
	FUNCTION_START

	FUNCTION_END
}

VOID WINAPI RW_RegisterStatic()
{
	FUNCTION_START

	FUNCTION_END
}

VOID WINAPI RW_RegisterDlg()
{
	FUNCTION_START

	FUNCTION_END
}

VOID WINAPI RW_RegisterEdit()
{
	FUNCTION_START

	FUNCTION_END
}

VOID WINAPI RW_RegisterLBoxCtl()
{
	FUNCTION_START

	FUNCTION_END
}

VOID WINAPI RW_RegisterSB()
{
	FUNCTION_START

	FUNCTION_END
}

VOID WINAPI RW_RegisterComboLBoxCtl()
{
	FUNCTION_START

	FUNCTION_END
}

VOID WINAPI RW_RegisterCBoxCtl()
{
	FUNCTION_START

	FUNCTION_END
}

VOID WINAPI RW_RegisteMDIClient()
{
	FUNCTION_START

	FUNCTION_END
}

VOID WINAPI LW_RegisterWindows(HINSTANCE USER_HeapSel)
{
	FUNCTION_START

	RW_RegisterMenus();
	RW_RegisterButton();
	RW_RegisterStatic();
	RW_RegisterDlg();
	RW_RegisterEdit();
	RW_RegisterLBoxCtl();
	RW_RegisterSB();
	RW_RegisterComboLBoxCtl();
	RW_RegisterCBoxCtl();
	RW_RegisteMDIClient();

	FUNCTION_END
}


/**********************************************************************************
 *
 * This is main initialization code of USER.EXE, same as LoadWindows functions in
 * original Windows
 *
 */

typedef
struct tagKBINFO {
  char kbRanges[4]; //Far East ranges for KANJI
  WORD kbStateSize; //#bytes of state info maintained by TOASCII
} KBINFO;

KBINFO KbInfo;

typedef
struct tagCURSORINFO {
	WORD dpXRate;		//horizontal mickey/pixel ratio
	WORD dpYRate;		//vertical mickey/pixel ratio
} CURSORINFO;

CURSORINFO CursorInfo;

WORD WINAPI InquireKeyboard(KBINFO FAR *KbInfo);

typedef
struct tagMOUSEINFO {
	BYTE msExists;	// true => mouse exists
	BYTE msRelative;  // true => relative coordinate
	WORD msNumButtons; // number of buttons on the mouse
	WORD msRate; // maximum rate of mouse input events
	WORD msXThresh; // threshold before acceleration
	WORD msYThresh;
	WORD msXRes;// x resolution
	WORD msYRes;// y resolution
} MOUSEINFO;

MOUSEINFO MouseInfo;

WORD WINAPI InquireMouse(MOUSEINFO FAR *MouseInfo);

WORD WINAPI InquireDisplay(CURSORINFO FAR *CursorInfo);

HANDLE WINAPI SetObjectOwner(HANDLE hObject, HANDLE hTask);

BOOL WINAPI MakeObjectPrivate(HANDLE hObject, BOOL bPrivate);

#define GetStockBrush(i) ((HBRUSH)GetStockObject(i))

#pragma off (unreferenced);
BOOL PASCAL LibMain( HINSTANCE hInstance/*, WORD wDataSegment, WORD wHeapSize, LPSTR lpszCmdLine*/ )
#pragma on (unreferenced);
{
	RECT rect = {0,0,100,100};
	HDC desktop;

	FUNCTION_START
	TRACE("inst=%04x", hInstance);

	USER_HeapSel=hInstance;

	LW_LoadSomeStrings();

	CBEntries=UT_GetIntFromProfile(IDS_TYPEAHEAD, 0x3c);
	DefQueueSize=UT_GetIntFromProfile(IDS_DEFAULTQUEUESIZE, 8);

	CreateQueue(DefQueueSize);

	LW_RegisterWindows(USER_HeapSel);

	TRACE("Keyboard init");
	if (InquireKeyboard(&KbInfo)==sizeof(KBINFO))
	{
		TRACE("Keyboard initdone");
		if (SetSpeed(-1)==-1) ;//return FALSE;	// ERROR
		//return TRUE;
	} else 
		FatalExit(0x0c);

	TRACE("Mouse init");
	if (InquireMouse(&MouseInfo))
	{
		TRACE("Mouse initdone");
	} else 
		FatalExit(0x0c);;

	TRACE("Display init");
	if (InquireDisplay(&CursorInfo)==sizeof(CURSORINFO))
	{
		TRACE("Display initdone");
	};

	TRACE("Create display context");
	tempHDC=CreateDC(DISPLAY, NULL, NULL, NULL);
	TRACE("Create display context done");
	DeleteDC(tempHDC);
	TRACE("3");
	TRACE("4");
	FatalExit(0x0c);
//	FillRect(desktop, &rect, GetStockBrush(WHITE_BRUSH));
	//TRACE("2");
	//DeleteDC(desktop);//ReleaseDC(0, desktop);
	//TRACE("3");
	
	FUNCTION_END
	return(1);
}

