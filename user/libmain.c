#include "class.h"
#include "dce.h"
#include "queue.h"
#include "menu.h"
#include "display.h"




int UT_GetIntFromProfile(UINT id, int defvalue)
{
	char section[30];
	char key[30];
	char value[30];
	int retVal;

	FUNCTION_START
	TRACE("id=%d, defvalue=%d", id, defvalue);

	retVal=defvalue;

	// Load name of section
	if (LoadString(USER_HeapSel, IDS_WINDOWS, section, sizeof(section)))
	{
		TRACE("section=%S", section);
		// Load name of key
		if (LoadString(USER_HeapSel, id, key, sizeof(key)))
		{
			TRACE("key=%S", key);
			// Check is non-empty value
			if (GetProfileString(section, key, "", value, sizeof(value)))
			{
				// Load value
				retVal=GetProfileInt(section, key, defvalue);
			}
		}
	}

	TRACE("retVal=%d", retVal);
	FUNCTION_END
	return retVal;
}

VOID WINAPI LW_LoadSomeStrings()
{
	FUNCTION_START
	LoadString(USER_HeapSel, IDS_SYSTEMERROR, szSysError, sizeof(szSysError));
	TRACE("szSysError=%s", szSysError);
	LoadString(USER_HeapSel, IDS_DIVIDEBYZERO, szDivZero, sizeof(szDivZero));
	TRACE("szDivZero=%s", szDivZero);
	LoadString(USER_HeapSel, IDS_UNTITLED, szUntitled, sizeof(szUntitled));
	TRACE("szUntitled=%s", szUntitled);
	LoadString(USER_HeapSel, IDS_ERROR, szError, sizeof(szError));
	TRACE("szError=%s", szError);
	LoadString(USER_HeapSel, IDS_OK, szOk, sizeof(szOk));
	TRACE("szOk=%s", szOk);
	LoadString(USER_HeapSel, IDS_CANCEL, szCancel, sizeof(szCancel));
	TRACE("szCancel=%s", szCancel);
	LoadString(USER_HeapSel, IDS_ABORT, szAbort, sizeof(szAbort));
	TRACE("szAbort=%s", szAbort);
	LoadString(USER_HeapSel, IDS_RETRY, szRetry, sizeof(szRetry));
	TRACE("szRetry=%s", szRetry);
	LoadString(USER_HeapSel, IDS_IGNORE, szIgnore, sizeof(szIgnore));
	TRACE("szIgnore=%s", szIgnore);
	LoadString(USER_HeapSel, IDS_YES, szYes, sizeof(szYes));
	TRACE("szYes=%s", szYes);
	LoadString(USER_HeapSel, IDS_NO, szNo, sizeof(szNo));
	TRACE("szNo=%s", szNo);
	// LoadString(USER_HeapSel, 0x4b, szClose, sizeof(szClose)); not found in user.exe resources
	LoadString(USER_HeapSel, IDS_AM, szAm, sizeof(szAm));
	TRACE("szAm=%s", szAm);
	LoadString(USER_HeapSel, IDS_PM, szPm, sizeof(szPm));
	TRACE("szPm=%s", szPm);
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

VOID WINAPI LW_RegisterWindows()
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

// Check Windows Internals for some info
VOID WINAPI LW_DriversInit()
{
	FUNCTION_START

	QUEUE_CreateSysMsgQueue(CBEntries);

	TRACE("Keyboard init");
	if (InquireKeyboard(&KbInfo)==sizeof(KBINFO))
	{
		TRACE("Keyboard initdone");
		if (SetSpeed(-1)==-1) FatalExit(0x0c);
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

	FUNCTION_END
}

VOID WINAPI LW_BrushInit()
{
	FUNCTION_START

	FUNCTION_END
}

// See Windows Internals
VOID WINAPI LW_LoadResources()
{
	char szWindows[0x14];
	char szBeep[0x14];
	char szSwapMouseButtons[0x14];

	FUNCTION_START

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
	TRACE("TEST");
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

	FUNCTION_END
}

#if 0
VOID WINAPI LW_OEMDependentInit()
{
	HDC hdc;
	HRSRC hRes;
	HGLOBAL hData;
	DISPLAY_CONFIG FAR* lpConfig; 
	FUNCTION_START

	HInstanceDisplay=GetModuleHandle(DISPLAY);

	/* Ищем ресурс config.bin (ID 1) @todo some drivers has ID=2001 */
	hRes = FindResource(HInstanceDisplay, MAKEINTRESOURCE(1), "oembin");
	if (!hRes) {
		FreeLibrary(HInstanceDisplay);
		return;
	}
    
	hData = LoadResource(HInstanceDisplay, hRes);
	if (!hData) {
		FreeLibrary(HInstanceDisplay);
		return;
	}
    
	lpConfig = (DISPLAY_CONFIG FAR *)LockResource(hData);
	if (!lpConfig) {
		FreeLibrary(HInstanceDisplay);
		return;
	}

	DumpDisplayConfig(lpConfig);
#endif
#if 0    
	/* Обновляем системные метрики */
	SysMetricsDef[SM_CYVTHUMB] = pConfig->cyVThumb;         /* Индекс 9 */
	SysMetricsDef[SM_CXHTHUMB] = pConfig->cxHThumb;         /* Индекс 10 */
	SysMetricsDef[SM_CYKANJIWINDOW] = pConfig->cyKanjiWindow; /* Индекс 18 */
	SysMetricsDef[SM_CXBORDER] = pConfig->cxBorder;         /* Индекс 5 */
	SysMetricsDef[SM_CYBORDER] = pConfig->cyBorder;         /* Индекс 6 */
    
	/* Применяем коэффициенты сжатия к размерам иконок и курсоров */
	if (pConfig->cxIconCompression > 0) {
		SysMetricsDef[SM_CXICON] = 32 / pConfig->cxIconCompression; /* Индекс 11 */
		SysMetricsDef[SM_CYICON] = 32 / pConfig->cyIconCompression; /* Индекс 12 */
	}
    
	if (pConfig->cxCursorCompression > 0) {
		SysMetricsDef[SM_CXCURSOR] = 32 / pConfig->cxCursorCompression; /* Индекс 13 */
		SysMetricsDef[SM_CYCURSOR] = 32 / pConfig->cyCursorCompression; /* Индекс 14 */
	}
    
	/* Обновляем системные цвета */
	SysColors[COLOR_SCROLLBAR] = RGB(pConfig->scrollbarR, pConfig->scrollbarG, pConfig->scrollbarB);
	SysColors[COLOR_BACKGROUND] = RGB(pConfig->desktopR, pConfig->desktopG, pConfig->desktopB);
	SysColors[COLOR_ACTIVECAPTION] = RGB(pConfig->activeCaptionR, pConfig->activeCaptionG, pConfig->activeCaptionB);
	SysColors[COLOR_INACTIVECAPTION] = RGB(pConfig->inactiveCaptionR, pConfig->inactiveCaptionG, pConfig->inactiveCaptionB);
	SysColors[COLOR_MENU] = RGB(pConfig->menuR, pConfig->menuG, pConfig->menuB);
	SysColors[COLOR_WINDOW] = RGB(pConfig->windowR, pConfig->windowG, pConfig->windowB);
	SysColors[COLOR_WINDOWFRAME] = RGB(pConfig->windowFrameR, pConfig->windowFrameG, pConfig->windowFrameB);
	SysColors[COLOR_MENUTEXT] = RGB(pConfig->menuTextR, pConfig->menuTextG, pConfig->menuTextB);
	SysColors[COLOR_WINDOWTEXT] = RGB(pConfig->windowTextR, pConfig->windowTextG, pConfig->windowTextB);
	SysColors[COLOR_CAPTIONTEXT] = RGB(pConfig->captionTextR, pConfig->captionTextG, pConfig->captionTextB);
	SysColors[COLOR_ACTIVEBORDER] = RGB(pConfig->activeBorderR, pConfig->activeBorderG, pConfig->activeBorderB);
	SysColors[COLOR_INACTIVEBORDER] = RGB(pConfig->inactiveBorderR, pConfig->inactiveBorderG, pConfig->inactiveBorderB);
	SysColors[COLOR_APPWORKSPACE] = RGB(pConfig->appWorkspaceR, pConfig->appWorkspaceG, pConfig->appWorkspaceB);
	SysColors[COLOR_HIGHLIGHT] = RGB(pConfig->hiliteBkR, pConfig->hiliteBkG, pConfig->hiliteBkB);
	SysColors[COLOR_HIGHLIGHTTEXT] = RGB(pConfig->hiliteTextR, pConfig->hiliteTextG, pConfig->hiliteTextB);
	SysColors[COLOR_BTNFACE] = RGB(pConfig->btnFaceR, pConfig->btnFaceG, pConfig->btnFaceB);
	SysColors[COLOR_BTNSHADOW] = RGB(pConfig->btnShadowR, pConfig->btnShadowG, pConfig->btnShadowB);
	SysColors[COLOR_GRAYTEXT] = RGB(pConfig->grayTextR, pConfig->grayTextG, pConfig->grayTextB);
	SysColors[COLOR_BTNTEXT] = RGB(pConfig->btnTextR, pConfig->btnTextG, pConfig->btnTextB);
    
	/* Также обновляем соответствующие объекты GDI (кисти, перья) */
	SYSCOLOR_SetColor(COLOR_SCROLLBAR, SysColors[COLOR_SCROLLBAR]);
	SYSCOLOR_SetColor(COLOR_BACKGROUND, SysColors[COLOR_BACKGROUND]);
	SYSCOLOR_SetColor(COLOR_ACTIVECAPTION, SysColors[COLOR_ACTIVECAPTION]);
	SYSCOLOR_SetColor(COLOR_INACTIVECAPTION, SysColors[COLOR_INACTIVECAPTION]);
	SYSCOLOR_SetColor(COLOR_MENU, SysColors[COLOR_MENU]);
	SYSCOLOR_SetColor(COLOR_WINDOW, SysColors[COLOR_WINDOW]);
	SYSCOLOR_SetColor(COLOR_WINDOWFRAME, SysColors[COLOR_WINDOWFRAME]);
	SYSCOLOR_SetColor(COLOR_MENUTEXT, SysColors[COLOR_MENUTEXT]);
	SYSCOLOR_SetColor(COLOR_WINDOWTEXT, SysColors[COLOR_WINDOWTEXT]);
	SYSCOLOR_SetColor(COLOR_CAPTIONTEXT, SysColors[COLOR_CAPTIONTEXT]);
	SYSCOLOR_SetColor(COLOR_ACTIVEBORDER, SysColors[COLOR_ACTIVEBORDER]);
	SYSCOLOR_SetColor(COLOR_INACTIVEBORDER, SysColors[COLOR_INACTIVECAPTION]);
	SYSCOLOR_SetColor(COLOR_APPWORKSPACE, SysColors[COLOR_APPWORKSPACE]);
	SYSCOLOR_SetColor(COLOR_HIGHLIGHT, SysColors[COLOR_HIGHLIGHT]);
	SYSCOLOR_SetColor(COLOR_HIGHLIGHTTEXT, SysColors[COLOR_HIGHLIGHTTEXT]);
	SYSCOLOR_SetColor(COLOR_BTNFACE, SysColors[COLOR_BTNFACE]);
	SYSCOLOR_SetColor(COLOR_BTNSHADOW, SysColors[COLOR_BTNSHADOW]);
	SYSCOLOR_SetColor(COLOR_GRAYTEXT, SysColors[COLOR_GRAYTEXT]);
	SYSCOLOR_SetColor(COLOR_BTNTEXT, SysColors[COLOR_BTNTEXT]);
#endif    
#if 0
	FreeLibrary(HInstanceDisplay);

//	SYSCOLOR_Init();

	hdc=GetDC(0);
	CXScreen=GetDeviceCaps(hdc, HORZRES);
	CYScreen=GetDeviceCaps(hdc, VERTRES);
	ReleaseDC(0, hdc);

	FUNCTION_END
}
#endif 

VOID WINAPI LW_OEMCursorInit()
{
	FUNCTION_START
	FUNCTION_END
}

VOID WINAPI InitSizeBorderDimensions()
{
	FUNCTION_START
	FUNCTION_END
}

VOID WINAPI LW_MouseInit()
{
	FUNCTION_START
	FUNCTION_END
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
	FUNCTION_START

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

	FUNCTION_END
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
	FUNCTION_START

	// Initialize default main window size (often a fraction of the screen size)
	CXSize = CXScreen / 2;
	CYSize = CYScreen / 2;  

	FUNCTION_END
}

LRESULT WINAPI SwitchWndProc(HWND hWnd, UINT wMsg, WPARAM wParam, LPARAM lParam)
{
//    LOGSTR((LF_API,"DeskTopWndProc(hWnd=%.04x,wMsg=%.04x,wParam=%x,lParam=%x)\n",
//	hWnd,wMsg,wParam,lParam));

	FUNCTION_START
//	TRACE("switchwnd_msg=0x%04X", wMsg);
	return DefWindowProc(hWnd, wMsg, wParam, lParam);
	FUNCTION_END
    return(1);
}


LRESULT WINAPI TitleWndProc(HWND hWnd, UINT wMsg, WPARAM wParam, LPARAM lParam)
{
//    LOGSTR((LF_API,"DeskTopWndProc(hWnd=%.04x,wMsg=%.04x,wParam=%x,lParam=%x)\n",
//	hWnd,wMsg,wParam,lParam));

	FUNCTION_START
	FUNCTION_END
    return(1L);
}

VOID WINAPI LW_InitWndMgr(HINSTANCE hInstance)
{
	WNDCLASS * pWndClass; // For use in registering classes
	HLOCAL hWndClass;

	FUNCTION_START

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

#if 0
{
	HDC tempHDC;
	HPEN hPenBlue;
	HBRUSH hBrushRed;

//	TRACE("Create display context");
	tempHDC=GetDC(0);
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
#endif

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

	FUNCTION_END
}

VOID WINAPI LW_DisplayDriverInit()
{
	HDC hDC;
	BOOL FOnBoardBitmap;

	FUNCTION_START

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
	FUNCTION_END
}

VOID WINAPI LW_LoadFonts(VOID)
{
	FUNCTION_START
	FUNCTION_END
}

VOID WINAPI LW_DesktopIconInit(VOID)
{
	FUNCTION_START
	FUNCTION_END
}


VOID WINAPI LW_DrawIconInit(VOID)
{
	FUNCTION_START
	FUNCTION_END
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
	// 0x48 -> "BOOT", 0x4F -> "TASKMAN.EXE, 0x4A -> "SYSTEM.INI"
	LoadString( USER_HeapSel, 0x48, szBoot, 0xA );
	LoadString( USER_HeapSel, 0x4F, szTaskMan, 0xD );
	LoadString( USER_HeapSel, 0x4A, szSysIni, 0x14 );
	
	// Get 0x82 bytes for use as the string buffer in the call to
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
	
	// Screen saver active? 0x64 = "ScreenSaveActive"
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
//	if ( ReadMessage(hmemSysMsgQueue, &msg, 0, 0, 0xFFfF, 1))
	goto EmptyMessages;
}

/**********************************************************************************
 *
 * This is main initialization code of USER.EXE, same as LoadWindows functions in
 * original Windows
 *
 */

BOOL PASCAL LibMain(HINSTANCE hInstance)
{
	TRACE("Windows user interface implementation.\r\n"
		"osFree Janus project (c) 2026 osFree\r\n"
		"GNU LGPL v2.1 or later");

	FUNCTION_START
	TRACE("hInstance=%x", hInstance);

        // Save the module and instance handles away in global vars
	USER_HeapSel=hInstance;
	// Don't use MK_FP here. For 0:xxxx far pointer 0 will be replaced by DS segment. Use MAKELP which preveny it.
	HModuleWin = GetModuleHandle(MAKELP(0, hInstance));

	// Loads USER strings variables from resources
	LW_LoadSomeStrings();

        // Get the number of entries in the system message queue and mul to 2
	CBEntries=UT_GetIntFromProfile(IDS_TYPEAHEAD, 0x3c) << 1;
        // Get the default number of messages in a task queue
	DefQueueSize=UT_GetIntFromProfile(IDS_DEFAULTQUEUESIZE, 8);

        // Create an application message queue. This is needed to create windows.
	SetMessageQueue(DefQueueSize);

	// GDI module handle
	hGDI = LoadLibrary( "GDI.EXE" );

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
	DCE_Init();

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
//	LW_OEMDependentInit();
	DISPLAY_Init();
	SYSCOLOR_Init();

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
	hwndFocus = 0; 

	// Loads values into the RGWSYSMet array.
	// These values can be retrieved via the
	// GetSystemMetrics() API
	LW_InitSysMetrics();

	// Create the global atom table.
	GlobalInitAtom();

	hbitmapClose = LoadBitmap(0, MAKEINTRESOURCE(OBM_CLOSE));
	hbitmapMinimize = LoadBitmap(0, MAKEINTRESOURCE(OBM_REDUCE));
	hbitmapMinimizeD = LoadBitmap(0, MAKEINTRESOURCE(OBM_REDUCED));
	hbitmapMaximize = LoadBitmap(0, MAKEINTRESOURCE(OBM_ZOOM));
	hbitmapMaximizeD = LoadBitmap(0, MAKEINTRESOURCE(OBM_ZOOMD));
	hbitmapRestore = LoadBitmap(0, MAKEINTRESOURCE(OBM_RESTORE));
	hbitmapRestoreD  = LoadBitmap(0, MAKEINTRESOURCE(OBM_RESTORED));


//	MENU_Init();

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
	SetCursorPos(CXScreen / 2, CYScreen / 2);

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


	// Load the "system" menu ("Restore", "Move", "Size", etc.)
	HSysMenu = LoadMenu(USER_HeapSel, MAKEINTRESOURCE(IDM_SYSMENU));

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

