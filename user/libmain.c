#include "class.h"
#include "dce.h"
#include "queue.h"

int UT_GetIntFromProfile(UINT id, int defvalue)
{
	char section[30];
	char key[30];
	char value[30];

//	FUNCTION_START

	if (!LoadString(USER_HeapSel, IDS_WINDOWS, section, sizeof(section)))
	{
		TRACE("error1");
		for(;;);
	}
	if (!LoadString(USER_HeapSel, id, key, sizeof(key)))
	{
		TRACE("error2");
		for(;;);
	}

	// Protect from empty value
	if (!GetProfileString(section, key, "", value, sizeof(value)))
	{
//		FUNCTION_END
		return defvalue;
	}

//	FUNCTION_END
	return GetProfileInt(section, key, defvalue);
}

#if 0

VOID CreateQueue(int QueueSize)
{
	LPQUEUE lpQueue;
	int size;

//	FUNCTION_START

	size=sizeof(QUEUE)+(QueueSize-1)*sizeof(INTERNALMSG);

	lpQueue=(LPQUEUE)GlobalAlloc(GPTR+GMEM_SHARE, size);
	lpQueue->OwningTask=GetCurrentTask();
	lpQueue->MessageSize=sizeof(INTERNALMSG);
	lpQueue->Size=QueueSize;
	//lpQueue->ReadPtr=???;
	//lpQueue->WritePtr=???;
	lpQueue->ExpWinVersion=GetExeVersion();
	lpQueue->WakeBits=QS_SMPARAMSFREE;
//	lpQueue->flags=QF_INIT;

	SetTaskQueue(0, (HGLOBAL)lpQueue);

//	FUNCTION_END
}

#endif

VOID WINAPI LW_LoadSomeStrings()
{
//	FUNCTION_START
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
//	FUNCTION_END
}

VOID WINAPI RW_RegisterMenus()
{
//	FUNCTION_START

//	FUNCTION_END
}

VOID WINAPI RW_RegisterButton()
{
//	FUNCTION_START

//	FUNCTION_END
}

VOID WINAPI RW_RegisterStatic()
{
//	FUNCTION_START

//	FUNCTION_END
}

VOID WINAPI RW_RegisterDlg()
{
//	FUNCTION_START

//	FUNCTION_END
}

VOID WINAPI RW_RegisterEdit()
{
//	FUNCTION_START

//	FUNCTION_END
}

VOID WINAPI RW_RegisterLBoxCtl()
{
//	FUNCTION_START

//	FUNCTION_END
}

VOID WINAPI RW_RegisterSB()
{
//	FUNCTION_START

//	FUNCTION_END
}

VOID WINAPI RW_RegisterComboLBoxCtl()
{
//	FUNCTION_START

//	FUNCTION_END
}

VOID WINAPI RW_RegisterCBoxCtl()
{
//	FUNCTION_START

//	FUNCTION_END
}

VOID WINAPI RW_RegisteMDIClient()
{
//	FUNCTION_START

//	FUNCTION_END
}

VOID WINAPI LW_RegisterWindows()
{
//	FUNCTION_START

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

//	FUNCTION_END
}

// Check Windows Internals for some info
VOID WINAPI LW_DriversInit()
{
//	FUNCTION_START

	QUEUE_CreateSysMsgQueue(CBEntries);

//	TRACE("Keyboard init");
	if (InquireKeyboard(&KbInfo)==sizeof(KBINFO))
	{
//		TRACE("Keyboard initdone");
		if (SetSpeed(-1)==-1) FatalExit(0x0c);
	} else 
		FatalExit(0x0c);

//	TRACE("Mouse init");
	if (InquireMouse(&MouseInfo))
	{
//		TRACE("Mouse initdone");
	} else 
		FatalExit(0x0c);;

//	TRACE("Display init");
	if (InquireDisplay(&CursorInfo)==sizeof(CURSORINFO))
	{
//		TRACE("Display initdone");
	};

//	FUNCTION_END
}

VOID WINAPI LW_DCInit()
{
//	FUNCTION_START

	// Pass to dce.c
	DCE_Init();

//	FUNCTION_END
}

VOID WINAPI LW_BrushInit()
{
//	FUNCTION_START

//	FUNCTION_END
}

// See Windows Internals
VOID WINAPI LW_LoadResources()
{
	char szWindows[0x14];
	char szBeep[0x14];
	char szSwapMouseButtons[0x14];

//	FUNCTION_START

	// Get some strings out of the USER resources, for use as
	// section and key strings in GetProfilelnt() calLs
	LoadString(USER_HeapSel, IDS_WINDOWS, szWindows, sizeof(szWindows));
	LoadString(USER_HeapSel, IDS_SWAPMOUSEBUTTONS, szSwapMouseButtons, sizeof(szSwapMouseButtons));
	LoadString(USER_HeapSel, IDS_BEEP, szBeep, sizeof(szBeep));

	SetCaretBlinkTime(UT_GetIntFromProfile(IDS_CURSORBLINKRATE, 0x1F4)); // 4 = "CursorBlinkRate"

	SetDoubleClickTime(UT_GetIntFromProfile(IDS_DOUBLECLICKSPEED, 0));

	// The next three fetched vaLues are stored in static vars
	UT_GetIntFromProfile(IDS_DOUBLECLICKWIDTH, 4); // 0x61 = "DoubleClickWidth"
	UT_GetIntFromProfile(IDS_DOUBLECLICKHEIGHT, 4); // 0x62 = "DoubleClickHeight"
	UT_GetIntFromProfile(IDS_MENUDROPALIGNMENT, 0); // 0x62 = "MenuDropAlignment"


	// Get the delay times related to displaying menus.
	// 0x5E = "MenuShowDelay", 0x5F = "MenuHideDelay". The
	// defauLt vaLue is seemingLy reLated to whether the
	// WF_CPU286 fLag is set. Perhaps there's some sort of
	// timing issue?
	IDelayMenuShow = UT_GetIntFromProfile(IDS_MENUSHOWDELAY, defaultVal);
	IDelayMenuHide = UT_GetIntFromProfile(IDS_MENUHIDEDELAY, defaultVal);

	// Set the vaLue of FSwapButtons, which indicates if the Left
	// and right mouse buttons should be swapped
	GetProfileString(szWindows, szSwapMouseButtons,
			szNullString, &szSwapMouseButtons, 2);

	//FSwapButtons = some calculation involving szYes, the buffer
	//filled by GetProfiLeString(), and a call to AnsiLower().

	// Set the value of FBeep. Used by MessageBeep() to
	// determine if a sound shouLd be produced
	GetProfileString( szWindows, szBeep, szNullString,
		&szSwapMouseButtons, 2 );

	//FBeep = some calculation involving szYes, the buffer
	//filled by GetProfiLeString(), and a call to AnsiLower().

	// 0x6B = "DragFullWindows". Drag around the entire window
	// contents, rather than just the frame.
	FDragFullWindows = UT_GetIntFromProfile(IDS_DRAGFULLWINDOWS, 0);

	// 0x6F = "CoolSwitch". This has to do with fast ALT-TAB
	// switching between different tasks.
	FFastAltTab = UT_GetIntFromProfile(IDS_COOLSWITCH, 1);

	// Get the Grid granularity of the desktop. 0x50 = "Desktop"
	// 8 = "GridGranularity". It appears that szBeep[] and
	// szWindows[] are being reused, rather than creating
	// additionaL char arrays on the stack.
	LoadString(USER_HeapSel, IDS_DESKTOP, szWindows, sizeof(szWindows));
	LoadString(USER_HeapSel, IDS_GRIDGRANULARITY, szBeep, sizeof(szBeep));

	CXYGranularity = GetProfileInt(szWindows, szBeep, 0) << 3;
	if ( !CXYGranularity) // Make sure CXYGranularity is at
		CXYGranularity++; // least 1.

	// Load some heavily used cursors
	HCursNormal = LoadCursor(0, IDC_ARROW);
	HCursIBeam = LoadCursor(0, IDC_IBEAM);
	HCursUpArrow= LoadCursor(0, IDC_UPARROW);
	HIconSample = LoadIcon(0, IDI_APPLICATION);

	// Set the "resource handler" address for cursors & icons.
	// @todo Not clear... Befor was non-DIB cursors???
//	SetResourceHandler(USER_HeapSel, 1, LoadDIBCursorHandler);
//	SetResourceHandler(USER_HeapSel, 3, LoadDIBIconHandler);

	// The Windows Logo
	HIconWindows = LoadIcon(USER_HeapSel, MAKEINTRESOURCE(OCR_ICOCUR));


	// Load the "resizing" cursors (when cursor is over a border)
	HCursSizeNWSE=LoadCursor(0, IDC_SIZENWSE);
	HCursSizeNESW=LoadCursor(0, IDC_SIZENESW);
	HCursSizeNS=LoadCursor(0,IDC_SIZENS);
	HCursSizeWE=LoadCursor(0,IDC_SIZEWE);

	// Load some icons used by various diaLog boxes
	HIconHand = LoadIcon(0, MAKEINTRESOURCE(OIC_HAND));
	HIconQues = LoadIcon(0, MAKEINTRESOURCE(OIC_QUES));
	HIconBang = LoadIcon(0, MAKEINTRESOURCE(OIC_BANG));
	HIconNote = LoadIcon(0, MAKEINTRESOURCE(OIC_NOTE));

	// The "4 directions" icon
	HCursSizeAll = LoadCursor(USER_HeapSel, MAKEINTRESOURCE(OCR_SIZEALL));

//	FUNCTION_END
}

VOID WINAPI LW_OEMDependentInit()
{
	HDC hdc;
//	FUNCTION_START
	HInstanceDisplay=GetModuleHandle(DISPLAY);

	hdc=GetDC(0);
	CXScreen=GetDeviceCaps(hdc, HORZRES);
	CYScreen=GetDeviceCaps(hdc, VERTRES);
	ReleaseDC(0, hdc);

//	FUNCTION_END
}

VOID WINAPI LW_OEMCursorInit()
{
//	FUNCTION_START
//	FUNCTION_END
}

VOID WINAPI InitSizeBorderDimensions()
{
//	FUNCTION_START
//	FUNCTION_END
}

VOID WINAPI LW_MouseInit()
{
//	FUNCTION_START
//	FUNCTION_END
}

//When Windows first starts, USER calls the InquireCursor function to retrieve
//information about the cursor. It then sets a system timer to call the
//CheckCursor function on each timer interrupt and enables the mouse driver,
//allowing the Windows mouse-event routine to call MoveCursor at each mouse
//interrupt occurrence. USER and Windows applications subsequently set the
//shape of the cursor using SetCursor.

VOID WINAPI PASCAL Timer_Event(VOID){
	 //static count = 0; 
	 
	 //count ++ ;
	 //sendmessage(bScan);
	CheckCursor();
//	 printf("\n\rSYSTEM.drv Timer_Event()");
} 

VOID WINAPI EnableInput()
{
//	FUNCTION_START

	// See "Undocumented Windows"
	EnableSystemTimers();

//	printf("EnableSystemTimers()");
	if(!CreateSystemTimer(100, (FARPROC)Timer_Event ) ){
		printf(" CreateSystemTimer()  FAILED ! ");
	}

	//Initialize some memory starting at RGBKeyState to 0. Maybe
	//some sort of an array. Do the same for RGBAsyncKeyState.

	// KEYBOARD.2. See the DDK examples for source code for the
	// Enable() routines.
	KeyboardEnable((FARPROC)keybd_event, RGBKeyState);

	//CopyKeyState(); // copies keyboard state tables?

//	if ( some static variabLe)
		MouseEnable((FARPROC)mouse_event); // MOUSE.2

	// Look for the presence of a SOUND driver. If found, get
	// the address of its enabLe() function, and calL it.
	HModSound = GetModuleHandle("SOUND");
	if ( HModSound )
		lpfnSoundEnable = GetProcAddress(HModSound, "enable");

	if ( lpfnSoundEnable )
		lpfnSoundEnable();

	// Call WNetEnable() to initialize the network module,
	// if a network is present. See FarCallNetDriver() entry
	// in "Undocumented Windows," and "Windows Network
	// Programming" by Ralph Davis.

//	if (PNetlnfo && *(&PNetlnfo + 0x50) )
//		(&PNetlnfo + 0x50)(); // Call through a function pointer
			
	// Broadcast a message to the installable device drivers?
	// 2 = DRV_ENABLE??
//	InternalBroadcastDriverMessage(0, 2, 0, 0, 0, 0, 0, 4);

//	FUNCTION_END
}


BOOL WINAPI SetDeskWallPaper(LPSTR lpszBmpFileName)
{
	return TRUE;
}

/***********************************************************************
 *           SetDeskPattern   (USER.279)
 */
BOOL WINAPI SetDeskPattern(void)
{
    return 1;//SystemParametersInfo( SPI_SETDESKPATTERN, -1, NULL, FALSE );
}


void WINAPI SetMinMaxInfo(void) 
{  
    // Initialize default main window size (often a fraction of the screen size)
    CXSize = CXScreen / 2;
    CYSize = CYScreen / 2;  
}

LRESULT WINAPI SwitchWndProc(HWND hWnd, UINT wMsg, WPARAM wParam, LPARAM lParam)
{
//    LOGSTR((LF_API,"DeskTopWndProc(hWnd=%.04x,wMsg=%.04x,wParam=%x,lParam=%x)\n",
//	hWnd,wMsg,wParam,lParam));

//	FUNCTION_START
//	TRACE("switchwnd_msg=0x%04X", wMsg);
	return DefWindowProc(hWnd, wMsg, wParam, lParam);
//	FUNCTION_END
    return(1);
}


LRESULT WINAPI TitleWndProc(HWND hWnd, UINT wMsg, WPARAM wParam, LPARAM lParam)
{
//    LOGSTR((LF_API,"DeskTopWndProc(hWnd=%.04x,wMsg=%.04x,wParam=%x,lParam=%x)\n",
//	hWnd,wMsg,wParam,lParam));

//	FUNCTION_START
//	FUNCTION_END
    return(1L);
}

VOID WINAPI LW_InitWndMgr(HINSTANCE hInstance)
{
	WNDCLASS * pWndClass; // For use in registering classes
	HLOCAL hWndClass;

//	FUNCTION_START

	//InitiaLize the following variabLes from static variabLes:
	//CXSize, CYSize, CYCaption, CXBorder, CYBorder, CYHScroLL,
	//and CXVScroLL

	SetMinMaxInfo(); // Appears to initialize some static vars.
			 // Uses CXScreen, CYScreen, CXBorder, etc ...

	// Allocate 0x1A bytes.
	hWndClass = UserLocalAlloc(LT_USER_CLASS, LMEM_ZEROINIT, sizeof(WNDCLASS));
	pWndClass = (WNDCLASS *)LocalLock(hWndClass);

	// Register the DeskTop class
	pWndClass->lpszClassName = DESKTOP_CLASS_ATOM;
	pWndClass->hCursor = LoadCursor(0, IDC_ARROW);
	pWndClass->lpfnWndProc = DesktopWndProc;
	pWndClass->hInstance = hInstance;
	pWndClass->style = CS_GLOBALCLASS | CS_DBLCLKS;
	pWndClass->hbrBackground = 1;
	RegisterClass(pWndClass);

	// Register the "switch window" class
	pWndClass->lpszClassName = WINSWITCH_CLASS_ATOM;
	pWndClass->hCursor = LoadCursor(0, IDC_ARROW);
	pWndClass->lpfnWndProc = SwitchWndProc;
	pWndClass->hInstance = hInstance;
	pWndClass->hIcon = LoadIcon(NULL, IDI_APPLICATION); 
	pWndClass->style = CS_GLOBALCLASS /*| CS_SAVEBITS /*| CS_VREDRAW | CS_HREDRAW*/;
	pWndClass->hbrBackground = 5;
	RegisterClass( pWndClass );

	// Register the icon title cLass
	pWndClass->lpszClassName = MAKELP(0, 0x8004);
	pWndClass->hCursor = LoadCursor(0, IDC_ARROW);
	pWndClass->lpfnWndProc = TitleWndProc;
	pWndClass->hInstance = hInstance;
	pWndClass->style = CS_GLOBALCLASS;
	pWndClass->hbrBackground = 0;
	RegisterClass(pWndClass);

	LocalUnlock(hWndClass);
	LocalFree(hWndClass); // Don't need WNDCLASS anymore!


	WIN_CreateDesktopWindow();

	HWndSwitch = CreateWindowEx( 0, WINSWITCH_CLASS_ATOM,
		"test1", WS_OVERLAPPEDWINDOW /*WS_DISABLED | WS_POPUP*/, 50, 50, 150, 150,
		HWndDesktop, 0, 0/*hInstance*/, 0 );

//@todo hm... must not be created because no such class!!!
	HWndRealPopup = CreateWindowEx( 0, MAKELP(0, 0x8003),
		"test2", WS_OVERLAPPEDWINDOW /*WS_DISABLED | WS_POPUP*/, 100, 100, 200, 200,
		HWndDesktop, 0, 0/*hInstance*/, 0 );

	// Move the switch window to the center of the screen
//	SetWindowPos( HWndSwitch, 0xFFFF, 0, 0, 0, 0,
//		SWP_NOSIZE | SWP_NOMOVE | SWP_NOREDRAW|SWP_NOACTIVATE);

	ShowWindow(HWndSwitch, SW_SHOW);;
	ShowWindow(HWndRealPopup, SW_SHOW);;

// Ok. Now time to test message queue...
{
	MSG      msg;
	
	while (GetMessage(&msg, 0, 0, 0)) 
	{
		//TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
}

for(;;);

{
	HPEN hPenBlue;
	HBRUSH hBrushRed;
	HDC xhdc=GetDC(HWndDesktop);

	TRACE("xhdc=%x", xhdc);
	SetPixel(xhdc, 10, 10, RGB(255, 0, 0));
        hPenBlue=CreatePen(PS_SOLID, 2, RGB(0, 0, 255));
        SelectObject(xhdc, hPenBlue);
        hBrushRed=CreateSolidBrush(RGB(255, 0, 0));
        SelectObject(xhdc, hBrushRed);
        Rectangle(xhdc, 200, 0, 400, 200);

	DeleteObject(hPenBlue);
	DeleteObject(hBrushRed);
	ReleaseDC(HWndDesktop, xhdc);
}

//	HWndRealPopup = CreateWindowEx( 0, MAKELP(0, 0x8000), 0,
//		WS_POPUP, 0, 0, 0x64, 0x64, 0, 0,
//		hInstance, 0 ); // Pop-up menu???

	SetDeskPattern();
	SetDeskWallPaper((LPSTR)-1);

	// Set the wallpaper and pattern. Read names from the WIN.INI fiLe
	// Tell the desktop that the palette may have changed from
	// loading the wallpaper image.
	SendMessage(HWndDesktop, WM_SYSCOLORCHANGE, 0, 0);

//	Toggle a bit in the HWndDesktop flags //?

//	InvalidateDCCache( HWndDesktop, 0); What this do??

	// Force the entire desktop to be refreshed
	InvalidateRect(HWndDesktop, 0, 1);
	UpdateWindow(HWndDesktop);

//	FUNCTION_END
}

VOID WINAPI LW_DisplayDriverInit()
{
	HDC hDC;
	BOOL FOnBoardBitmap;

	// Need a device context below
	hDC = GetDC(0);

	// If the display driver can save bits, get a function ptr
	// to the routine that does it. The function has an entry
	// ordinal of 92 (0x5C)
	FOnBoardBitmap = GetDeviceCaps(hDC, RASTERCAPS) & 0x0040;
	if (FOnBoardBitmap)
		LpSaveBitmap=GetProcAddress(HInstanceDisplay,MAKELP(0,92));
	
	//Done with the device context
	ReleaseDC(0, hDC);

	// DISPLAY.500 -> UserRepaintDisable(). This function
	// tells the display driver when screen updates should be
	// enabled or disabled.
	LpDisplayCriticalSection =
		GetProcAddress( HInstanceDisplay, MAKELP(0, 500) );
}

VOID WINAPI LW_LoadFonts(VOID)
{
}

VOID WINAPI LW_DesktopIconInit(VOID)
{
}


VOID WINAPI LW_DrawIconInit(VOID)
{
}


VOID WINAPI LW_LoadTaskmanAndScreenSaver(VOID)
{
	WORD IScreenSaveTimeOut;  
	
	// Locals:
	char szBoot[0xA];
	char szTaskMan[0xD];
	char szSysIni[0x14];

    FUNCTION_START 
	// Get some strings out of the USER string tables.
	//Ox48 -> "BOOT", Ox4F -> "TASKMAN.EXE, Ox4A -> "SYSTEM.INI"
	LoadString( USER_HeapSel, 0x48, szBoot, 0xA );
	LoadString( USER_HeapSel, 0x4F, szTaskMan, 0xD );
	LoadString( USER_HeapSel, 0x4A, szSysIni, 0x14 );
	
	// Get Ox82 bytes for use as the string buffer in the call to
	// GetPrivateProfileString(), below.
	hTaskManName = UserLocalAlloc(LT_USER_STRING, 0x40, 0x82 );
	PTaskManName = (LPSTR)LocalLock(hTaskManName);

	// Get the "final" name of TASKMAN.EXE from the boot section
	// of the SYSTEM.INI file. The default is taskman.exe=taskman.exe.
	GetPrivateProfileString(szBoot, szTaskMan, szTaskMan, PTaskManName , 0x82, szSysIni );
	
	// Get rid of the excess memory that was allocated previously.
	LocalUnlock(hTaskManName);
	LocalReAlloc(hTaskManName, lstrlen(PTaskManName)+1, 0 );
	
	// The screen saver timeout value. Ox63 = "ScreenSaveTimeOut"
	IScreenSaveTimeOut = UT_GetIntFromProfile(0x63, 0);
	
	// Screen saver active? Ox64 = "ScreenSaveActive"
	if ( UT_GetIntFromProfile( 0x64, 0 ) == 0 )
	{
		if ( IScreenSaveTimeOut > 0 )
				IScreenSaveTimeOut = -IScreenSaveTimeOut; //???
	}
	
	FUNCTION_END
}

VOID WINAPI Disablelnput(VOID)
{
	FARPROC lpfnSoundDisable;
	MSG msg;

	// Broadcast message to i.nstaUable drivers.
	// 5 = DRV-DISABLE???
	//InternalBroadcastDriverMessage( 0, 5, 0, 0, 0, 0, 0, 6 )

	// Call WNetDisable(). See the entry for FarCallNetDriver()
	// in Undocumented Windows.
	// if ( PNetInfo && ( *(DWORD *)(PNetInfo + 0x54) ) )
//		(PNetlnfo + 0x54)() // Call through a function pointer

	if (HModSound)
	{
		// Disable the SOUND driver, if present.
		lpfnSoundDisable = GetProcAddress(HModSound, "disable");
		if ( lpfnSoundDisable )
			lpfnSoundDisable();
	}

	// Say goodbye to the mouse
	//if ( some static var )
	//DisableMouse();
	
	//Disable(); // Disable the keyboard driver

//	DisableSystemTimers(); // See "Undocumented Windows"
EmptyMessages:
	// Keep reading system messages till the system message queue is empty.
//	if ( ReadMessage(HQSysQueue, &msg, 0, 0, 0xFFfF, 1))
	goto EmptyMessages;
}

/**********************************************************************************
 *
 * This is main initialization code of USER.EXE, same as LoadWindows functions in
 * original Windows
 *
 */

BOOL PASCAL LibMain( HINSTANCE hInstance )
{
        HPEN hPenBlue;
        HBRUSH hBrushRed;
	RECT rect = {0,0,100,100};
	HDC desktop;

//	FUNCTION_START
//	TRACE("inst=%x", hInstance);

	hGDI = LoadLibrary( "GDI.EXE" );
 
        // Save the module and instance handles away in global vars
	USER_HeapSel=hInstance;
	HModuleWin = GetModuleHandle(MK_FP(0, hInstance));

	// Loads USER strings variables from resources
	LW_LoadSomeStrings();

        // Get the number of entries in the system message queue and mul to 2
	CBEntries=UT_GetIntFromProfile(IDS_TYPEAHEAD, 0x3c) << 1;
        // Get the default number of messages in a task queue
	DefQueueSize=UT_GetIntFromProfile(IDS_DEFAULTQUEUESIZE, 8);

        // Create an application message queue. This is needed to create windows.
	SetMessageQueue(DefQueueSize);
//	CreateQueue(DefQueueSize);

	// Get the default border width for a window. Default is 3.
	ClBorder = UT_GetIntFromProfile(IDS_BORDER, 3);
	if (ClBorder < 1) // Make sure it's got a reasonable
            ClBorder = 1; // value.
        if (ClBorder > 0x32 )
            ClBorder = 0x32;

	// Setup and initialize the Keyboard,
	// mouse, and COMM drivers. The system
	// message queue is created here (in
	// DI_EventInit(), which is called from
	// LW_DriversInit()).
	LW_DriversInit();

	// LW_DCInit() is where the 5 DISPLAY device contexts are
	// created. Chapter 5 (Windows Internals) covers device contexts (DCs) in
	// detail. LW_DCInit() also calls @InitCreateRgn(),
	// GetDCState(), CreateCompatibleDC(), and GetStockObject().
	// The HFontSys and HFontSysFixed global vars are set here.
	LW_DCInit();

	SYSCOLOR_Init();

	// Calls GetStockObject() to set the variables
	// HBrWhite and HBrBlack. Calls CreateBitmap(),
	// CreatePatternBrush(), DeleteObject(),
	// and MakeObjectPrivate()
	LW_BrushInit();

	// Perform various initialization required by the DISPLAY
	// driver. Sets HInstanceDisplay variable by calling
	// GetModuleHandle("DISPLAY"). LW_OemDependentInit() is a very
	// large function, with numerous calls to routines such as
	// GetDeviceCaps(), GetStockObject(), SetResourceHandler(),
	// FindResource(), ODI_CreateBits(), and so on.
	LW_OEMDependentInit();


	// Sets HBmCursorBitmap and HPermanentCursor variables.
	LW_OEMCursorInit();

	// Set the global variables: CLBorder, CXBorder, CXSzBorder,
	// CXSzBorderPlus1, CYBorder, CYSzBorder, CYSzBorderPlus1,
	// CXCwMargin, and CYCwMargin. Presumably these deal with
	// the size of the borders around Windows.
	InitSizeBorderDimensions();

	//Loads lots of icons and cursors.
	LW_LoadResources();

	// Start out with no focus
	HWndFocus = 0; 

	// Loads values into the RGWSYSMet array.
	// These values can be retrieved via the
	// GetSystemMetrics() API
	LW_InitSysMetrics();

	// Create the global atom table.
	GlobalInitAtom();

	hbitmapClose = LoadBitmap(0, MAKEINTRESOURCE(OBM_CLOSE));
	hbitmapMinimize  = LoadBitmap(0, MAKEINTRESOURCE(OBM_REDUCE));
	hbitmapMinimizeD = LoadBitmap(0, MAKEINTRESOURCE(OBM_REDUCED));
	hbitmapMaximize  = LoadBitmap(0, MAKEINTRESOURCE(OBM_ZOOM));
	hbitmapMaximizeD = LoadBitmap(0, MAKEINTRESOURCE(OBM_ZOOMD));
	hbitmapRestore   = LoadBitmap(0, MAKEINTRESOURCE(OBM_RESTORE));
	hbitmapRestoreD  = LoadBitmap(0, MAKEINTRESOURCE(OBM_RESTORED));

	// Register the windows classes for
	// "predefined" windows, such as
	// edit controls, etc. 
	LW_RegisterWindows();

	// Allocate some memory for an internal buffer.
	// UserLocalAlloc() is a special version of LocalAlloc().
	// The first parameter "marks" each item in the USER local
	// heap with a value that indicates what it is. This
	// "tagging" of blocks only occurs in the debug version of
	// USER. See the TOOLHELP LocalEntry documentation for
	// a list of the various "tag" values.
	//PState = UserLocalAlloc(LT_USER_STRING, 0x40, some_static_var + 0x10 )

	// Sets X_Mickey_Rate, Y_Mickey_Rate, and
	// calls ClipCursor( 0 )
	LW_MouseInit();

	// Call the "enable" routine for various
	// input/output devices.
	EnableInput();

	// Middle of the screen
	SetCursorPos(100, 100);

	// Get the hourglass cursor, show it
	SetCursor(LoadCursor(0, IDC_WAIT));

	// Register the Desktop and switch windows classes, and
	// create the windows.
	LW_InitWndMgr(hInstance);


	// Max button size???
	//WMaxBtnSize = MB_FindLongestString();
	
	// Why it was here in Pietrek book? Needed for RegisterClass. Create the global atom table.
//	GlobalInitAtom();

//	AtomCheckPointProp = GlobalAddAtom("SysCP") // Used by
//	AtomBwlProp = GlobalAddAtom( "SysBW" ) // EnumProps()

//	MsgWinHelp = RegisterWindowMessage( "WM_WINHELP");

	// Allocate another local heap for menus
	MenuBase = SELECTOROF(GlobalLock(HMenuHeap = GlobalAlloc( GMEM_DDESHARE | GMEM_MOVEABLE | GMEM_ZEROINIT, 0x418 )));

	// Allocate another local heap for menu strings
	MenuStringBase = SELECTOROF(GlobalLock(HMenuStringHeap = GlobalAlloc( GMEM_DDESHARE | GMEM_MOVEABLE | GMEM_ZEROINIT, 0x418 )));

	// Initialize the menu and menu string heaps. The heaps
	// start out small (0x417 bytes), but can grow as needed.
	LocalInit(MenuBase, 0x12, 0x417);
	LocalInit(MenuStringBase, 0x12, 0x417);


	// list registered classes
//	CLASS_WalkClasses();
{
//	TRACE("Create display context");
	tempHDC=CreateDC(DISPLAY, NULL, NULL, NULL);
//	TRACE("Create display context done");
	SetPixel(tempHDC, 10, 10, RGB(255, 0, 0));
        hPenBlue=CreatePen(PS_SOLID, 2, RGB(0, 0, 255));
        SelectObject(tempHDC, hPenBlue);
        hBrushRed=CreateSolidBrush(RGB(255, 0, 0));
        SelectObject(tempHDC, hBrushRed);
        Rectangle(tempHDC, 200, 0, 400, 200);

	DrawIcon(tempHDC, 205, 5, HCursNormal);
	DrawIcon(tempHDC, 250, 5, HCursIBeam);
	DrawIcon(tempHDC, 290, 5, HCursUpArrow);
	DrawIcon(tempHDC, 330, 5, HIconSample);

	DrawIcon(tempHDC, 205, 40, HIconWindows);
	DrawIcon(tempHDC, 250, 40, HCursSizeNWSE);
	DrawIcon(tempHDC, 290, 40, HCursSizeNESW);
	DrawIcon(tempHDC, 330, 40, HCursSizeNS);

	DrawIcon(tempHDC, 205, 80, HCursSizeWE);
	DrawIcon(tempHDC, 250, 80, HIconHand);
	DrawIcon(tempHDC, 290, 80, HIconQues);
	DrawIcon(tempHDC, 330, 80, HIconBang);

	DrawIcon(tempHDC, 205, 120, HIconNote);
	DrawIcon(tempHDC, 250, 120, HCursSizeAll);

//	DeleteDC(tempHDC);
}

for (;;);


	// Load the "system" menu ("Restore", "Move", "Size", etc.)
	HSysMenu = LoadMenu(USER_HeapSel, MK_FP(0, IDM_SYSMENU));

	LW_DisplayDriverInit(); // Gets entry points in display driver

	LW_LoadFonts();	// Uses AddFontResource() to load all the
			// fonts in the "fonts" section of WIN.INI

	LW_DesktopIconInit(); // Initialize things related to
				// desktop icons/fonts

	LW_DrawIconInit(); // Initializes HBmDrawIconMono and
				// HMbDrawIconColor

	LW_LoadTaskmanAndScreenSaver(); // Doesn't _load_ them. Just
					// gets configuration values


	FUNCTION_END
	return(1);
}

