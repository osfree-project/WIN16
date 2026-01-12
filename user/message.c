/*
 * Message queues related functions
 *
 * Copyright 1993, 1994 Alexandre Julliard
 */

//#include <stdlib.h>
//#include <sys/time.h>
//#include <sys/types.h>
//#include <errno.h>

//#include "message.h"
//#include "win.h"
//#include "gdi.h"
//#include "sysmetrics.h"
//#include "hook.h"
//#include "event.h"
//#include "spy.h"
//#include "winpos.h"
//#include "atom.h"
//#include "dde.h"
//#include "stddebug.h"
/* #define DEBUG_MSG */
#include "user.h"
#include "queue.h"

//#define HWND_BROADCAST  ((HWND)0xffff)

extern BYTE* 	KeyStateTable;				 /* event.c */
WPARAM	lastEventChar;				 /* event.c */

//extern BOOL TIMER_CheckTimer( LONG *next, MSG *msg,
//			      HWND hwnd, BOOL remove );  /* timer.c */

DWORD MSG_WineStartTicks;  				 /* Ticks at Wine startup */

static WORD doubleClickSpeed = 452;

int abs(int i)
{
  return i < 0 ? -i : i;
}

/***********************************************************************
 *           MSG_TranslateMouseMsg
 *
 * Translate an mouse hardware event into a real mouse message.
 * Return value indicates whether the translated message must be passed
 * to the user.
 * Actions performed:
 * - Find the window for this message.
 * - Translate button-down messages in double-clicks.
 * - Send the WM_NCHITTEST message to find where the cursor is.
 * - Activate the window if needed.
 * - Translate the message into a non-client message, or translate
 *   the coordinates to client coordinates.
 * - Send the WM_SETCURSOR message.
 */
static BOOL MSG_TranslateMouseMsg( MSG *msg, BOOL remove )
{
    WND *pWnd;
    BOOL eatMsg = FALSE;
    int hittest;
    static DWORD lastClickTime = 0;
    static WORD  lastClickMsg = 0;
    static POINT lastClickPos = { 0, 0 };
    POINT pt = msg->pt;
    //MOUSEHOOKSTRUCT hook = { msg->pt, 0, HTCLIENT, 0 };

    BOOL mouseClick = ((msg->message == WM_LBUTTONDOWN) ||
		       (msg->message == WM_RBUTTONDOWN) ||
		       (msg->message == WM_MBUTTONDOWN));

//FUNCTION_START
//TRACE("msg=0x%04X", msg->message);

      /* Find the window */

    if (GetCapture())
    {
	msg->hwnd = GetCapture();
	ScreenToClient( msg->hwnd, &pt );
	msg->lParam = MAKELONG( pt.x, pt.y );
        /* No need to further process the message */
        //hook.hwnd = msg->hwnd;
	TRACE("123");
	for(;;);
//        return !HOOK_CallHooks( WH_MOUSE, remove ? HC_ACTION : HC_NOREMOVE,
//                                msg->message, (LPARAM)MAKE_SEGPTR(&hook));
    }
   
    hittest = WINPOS_WindowFromPoint( msg->pt, &pWnd );
    msg->hwnd = pWnd->hwndSelf;
    if (hittest != HTERROR)
    {
        /* Send the WM_PARENTNOTIFY message */

        if (mouseClick) WIN_SendParentNotify( msg->hwnd, msg->message, 0,
                                            MAKELONG( msg->pt.x, msg->pt.y ) );

        /* Activate the window if needed */
        if (mouseClick)
        {
            HWND hwndTop = WIN_GetTopParent( msg->hwnd );

//		TRACE("MOUSE CLICK");

            if (hwndTop != GetActiveWindow())
            {
                LONG ret = SendMessage( msg->hwnd, WM_MOUSEACTIVATE,
					(WPARAM)hwndTop,
                                        MAKELONG( hittest, msg->message ) );
                if ((ret == MA_ACTIVATEANDEAT) || (ret == MA_NOACTIVATEANDEAT))
                    eatMsg = TRUE;
                if ((ret == MA_ACTIVATE) || (ret == MA_ACTIVATEANDEAT))
                {
                    SetWindowPos( hwndTop, HWND_TOP, 0, 0, 0, 0,
                                 SWP_NOSIZE | SWP_NOMOVE | SWP_NOACTIVATE );
                    WINPOS_ChangeActiveWindow( hwndTop, TRUE );
                }
            }
        }
    }

      /* Send the WM_SETCURSOR message */

    SendMessage( msg->hwnd, WM_SETCURSOR, (WPARAM)msg->hwnd,
                 MAKELONG( hittest, msg->message ));
    if (eatMsg) return FALSE;

      /* Check for double-click */

    if (mouseClick)
    {
	BOOL dbl_click = FALSE;

	if ((msg->message == lastClickMsg) &&
	    (msg->time - lastClickTime < doubleClickSpeed) &&
	    (abs(msg->pt.x - lastClickPos.x) < GetSystemMetrics(SM_CXDOUBLECLK)/2) &&
	    (abs(msg->pt.y - lastClickPos.y) < GetSystemMetrics(SM_CYDOUBLECLK)/2))
	    dbl_click = TRUE;

	if (dbl_click && (hittest == HTCLIENT))
	{
	    /* Check whether window wants the double click message. */
            dbl_click = (WIN_CLASS_STYLE(pWnd) & CS_DBLCLKS) != 0;
	}

	if (dbl_click) switch(msg->message)
	{
	    case WM_LBUTTONDOWN: msg->message = WM_LBUTTONDBLCLK; break;
	    case WM_RBUTTONDOWN: msg->message = WM_RBUTTONDBLCLK; break;
	    case WM_MBUTTONDOWN: msg->message = WM_MBUTTONDBLCLK; break;
	}

	if (remove)
	{
	    lastClickTime = msg->time;
	    lastClickMsg  = msg->message;
	    lastClickPos  = msg->pt;
	}
    }

      /* Build the translated message */

    if (hittest == HTCLIENT)
        ScreenToClient( msg->hwnd, &pt );
    else
    {
	msg->wParam = hittest;
	msg->message += WM_NCLBUTTONDOWN - WM_LBUTTONDOWN;
    }
    msg->lParam = MAKELONG( pt.x, pt.y );
    
//    hook.hwnd = msg->hwnd;
//    hook.wHitTestCode = hittest;
//TRACE("321");
//for(;;);
	return TRUE; 
//    return !HOOK_CallHooks( WH_MOUSE, remove ? HC_ACTION : HC_NOREMOVE,
//                            msg->message, (LPARAM)MAKE_SEGPTR(&hook));
}


/***********************************************************************
 *           MSG_TranslateKeyboardMsg
 *
 * Translate an keyboard hardware event into a real message.
 * Return value indicates whether the translated message must be passed
 * to the user.
 */
static BOOL MSG_TranslateKeyboardMsg( MSG *msg, BOOL remove )
{
      /* Should check Ctrl-Esc and PrintScreen here */

    msg->hwnd = GetFocus();
    if (!msg->hwnd)
    {
	  /* Send the message to the active window instead,  */
	  /* translating messages to their WM_SYS equivalent */

	msg->hwnd = GetActiveWindow();

	if( msg->message < WM_SYSKEYDOWN )
	    msg->message += WM_SYSKEYDOWN - WM_KEYDOWN;
    }
TRACE("234");
for(;;);
//    return !HOOK_CallHooks( WH_KEYBOARD, remove ? HC_ACTION : HC_NOREMOVE,
//                            msg->wParam, msg->lParam );
}


/***********************************************************************
 *           MSG_PeekHardwareMsg
 *
 * Peek for a hardware message matching the hwnd and message filters.
 */
static BOOL MSG_PeekHardwareMsg( MSG *msg, HWND hwnd, WORD first, WORD last,
                                 BOOL remove )
{
    MESSAGEQUEUE FAR *sysMsgQueue = QUEUE_GetSysQueue();
    int i, pos = sysMsgQueue->nextMessage;

    for (i = 0; i < sysMsgQueue->msgCount; i++, pos++)
    {
        if (pos >= sysMsgQueue->queueSize) pos = 0;
	*msg = sysMsgQueue->messages[pos].msg;

          /* Translate message */
        if ((msg->message >= WM_MOUSEFIRST) && (msg->message <= WM_MOUSELAST))
        {
            if (!MSG_TranslateMouseMsg( msg, remove )) continue;
        }
        else if ((msg->message >= WM_KEYFIRST) && (msg->message <= WM_KEYLAST))
        {
            if (!MSG_TranslateKeyboardMsg( msg, remove )) continue;
        }
        else  /* Non-standard hardware event */
        {
//            HARDWAREHOOKSTRUCT hook = { msg->hwnd, msg->message,
//                                        msg->wParam, msg->lParam };
TRACE("432");
for(;;);
//            if (HOOK_CallHooks( WH_HARDWARE, remove ? HC_ACTION : HC_NOREMOVE,
//                                0, (LPARAM)MAKE_SEGPTR(&hook) )) continue;
        }

          /* Check message against filters */

        if (hwnd && (msg->hwnd != hwnd)) continue;
        if ((first || last) && 
            ((msg->message < first) || (msg->message > last))) continue;
        if ((msg->hwnd != GetDesktopWindow()) && 
            (GetWindowTask(msg->hwnd) != GetCurrentTask()))
            continue;  /* Not for this task */
        if (remove)
        {
            MSG tmpMsg = *msg; /* FIXME */
//            HOOK_CallHooks( WH_JOURNALRECORD, HC_ACTION,
//                            0, (LPARAM)MAKE_SEGPTR(&tmpMsg) );
            QUEUE_RemoveMsg( sysMsgQueue, pos );
        }
        return TRUE;
    }
    return FALSE;
}


/**********************************************************************
 *           SetDoubleClickTime   (USER.20)
 */
void WINAPI SetDoubleClickTime( UINT interval )
{
    doubleClickSpeed = interval ? interval : 500;
}		


/**********************************************************************
 *           GetDoubleClickTime   (USER.21)
 */
UINT WINAPI GetDoubleClickTime()
{
    return doubleClickSpeed;
}		


/***********************************************************************
 *           MSG_GetHardwareMessage
 *
 * Like GetMessage(), but only return mouse and keyboard events.
 * Used internally for window moving and resizing. Mouse messages
 * are not translated.
 * Warning: msg->hwnd is always 0.
 */
BOOL MSG_GetHardwareMessage( LPMSG msg )
{
    int pos;
//    XEvent event;
    MESSAGEQUEUE FAR *sysMsgQueue = QUEUE_GetSysQueue();

    while(1)
    {    
	if ((pos = QUEUE_FindMsg( sysMsgQueue, 0, 0, 0 )) != -1)
	{
	    *msg = sysMsgQueue->messages[pos].msg;
	    QUEUE_RemoveMsg( sysMsgQueue, pos );
	    break;
	}
//	XNextEvent( display, &event );
//	EVENT_ProcessEvent( &event );
    }
    return TRUE;
}

#if 0

/***********************************************************************
 *           MSG_Synchronize
 *
 * Synchronize with the X server. Should not be used too often.
 */
void MSG_Synchronize()
{
    XEvent event;

    XSync( display, False );
    while (XPending( display ))
    {
	XNextEvent( display, &event );
	EVENT_ProcessEvent( &event );
    }    
}

/***********************************************************************
 *           MSG_WaitXEvent
 *
 * Wait for an X event, but at most maxWait milliseconds (-1 for no timeout).
 * Return TRUE if an event is pending, FALSE on timeout or error
 * (for instance lost connection with the server).
 */
BOOL MSG_WaitXEvent( LONG maxWait )
{
    fd_set read_set;
    struct timeval timeout;
    XEvent event;
    int fd = ConnectionNumber(display);

    if (!XPending(display) && (maxWait != -1))
    {
        FD_ZERO( &read_set );
        FD_SET( fd, &read_set );

	timeout.tv_usec = (maxWait % 1000) * 1000;
	timeout.tv_sec = maxWait / 1000;

#ifdef CONFIG_IPC
	sigsetjmp(env_wait_x, 1);
	stop_wait_op= CONT;
	    
	if (DDE_GetRemoteMessage()) {
	    while(DDE_GetRemoteMessage())
		;
	    return TRUE;
	}
	stop_wait_op= STOP_WAIT_X;
	/* The code up to the next "stop_wait_op= CONT" must be reentrant  */
	if (select( fd+1, &read_set, NULL, NULL, &timeout ) != 1 &&
	    !XPending(display)) {
	    stop_wait_op= CONT;
	    return FALSE;
	} else {
	    stop_wait_op= CONT;
	}
#else  /* CONFIG_IPC */
	if (select( fd+1, &read_set, NULL, NULL, &timeout ) != 1)
            return FALSE;  /* Timeout or error */
#endif  /* CONFIG_IPC */

    }

    /* Process the event (and possibly others that occurred in the meantime) */
    do
    {

#ifdef CONFIG_IPC
	if (DDE_GetRemoteMessage())
        {
	    while(DDE_GetRemoteMessage()) ;
	    return TRUE;
	}
#endif  /* CONFIG_IPC */

        XNextEvent( display, &event );
        EVENT_ProcessEvent( &event );
    }
    while (XPending( display ));
    return TRUE;
}
#endif

/***********************************************************************
 *           MSG_PeekMessage
 */
static BOOL MSG_PeekMessage( LPMSG msg, HWND hwnd, WORD first, WORD last,
                             WORD flags, BOOL peek )
{
    int pos, mask;
    MESSAGEQUEUE FAR *msgQueue;
    HQUEUE	  hQueue;
    LONG nextExp;  /* Next timer expiration time */

//	FUNCTION_START
#ifdef CONFIG_IPC
    DDE_TestDDE(hwnd);	/* do we have dde handling in the window ?*/
    DDE_GetRemoteMessage();
#endif  /* CONFIG_IPC */

    
    if (first || last)
    {
	mask = QS_POSTMESSAGE;  /* Always selectioned */
	if ((first <= WM_KEYLAST) && (last >= WM_KEYFIRST)) mask |= QS_KEY;
	if ((first <= WM_MOUSELAST) && (last >= WM_MOUSEFIRST)) mask |= QS_MOUSE;
	if ((first <= WM_TIMER) && (last >= WM_TIMER)) mask |= QS_TIMER;
//@todo fix?	if ((first <= WM_SYSTIMER) && (last >= WM_SYSTIMER)) mask |= QS_TIMER;
	if ((first <= WM_PAINT) && (last >= WM_PAINT)) mask |= QS_PAINT;
    }
    else mask = QS_MOUSE | QS_KEY | QS_POSTMESSAGE | QS_TIMER | QS_PAINT;

    while(1/*1*/)
    {    
	hQueue   = GetTaskQueue(0);
        msgQueue = (MESSAGEQUEUE FAR *)GlobalLock( hQueue );
        if (!msgQueue) return FALSE;


	  /* First handle a message put by SendMessage() */
	if (msgQueue->status & QS_SENDMESSAGE)
	{
		TRACE("SendMessage");
	    if (!hwnd || (msgQueue->hWnd == hwnd))
	    {
		if ((!first && !last) || 
		    ((msgQueue->msg >= first) && (msgQueue->msg <= last)))
		{
		    msg->hwnd    = msgQueue->hWnd;
		    msg->message = msgQueue->msg;
		    msg->wParam  = msgQueue->wParam;
		    msg->lParam  = msgQueue->lParam;
		    if (flags & PM_REMOVE) msgQueue->status &= ~QS_SENDMESSAGE;
		    break;
		}
	    }
	}
    
	  /* Now find a normal message */
	pos = QUEUE_FindMsg( msgQueue, hwnd, first, last );

	if (pos != -1)
	{
	    QMSG *qmsg = &msgQueue->messages[pos];
	    *msg = qmsg->msg;
	    msgQueue->GetMessageTimeVal      = msg->time;
	    msgQueue->GetMessagePosVal       = *(DWORD *)&msg->pt;
	    msgQueue->GetMessageExtraInfoVal = qmsg->extraInfo;

	    if (flags & PM_REMOVE) QUEUE_RemoveMsg( msgQueue, pos );
	    break;
	}

	  /* Now find a hardware event */
        if (MSG_PeekHardwareMsg( msg, hwnd, first, last, flags & PM_REMOVE ))
        {
//TRACE("mouse!! %d", hwnd);
            /* Got one */
	    msgQueue->GetMessageTimeVal      = msg->time;
	    msgQueue->GetMessagePosVal       = *(DWORD *)&msg->pt;
	    msgQueue->GetMessageExtraInfoVal = 0;  /* Always 0 for now */
            break;
        }

	  /* Now handle a WM_QUIT message */
	if (msgQueue->wPostQMsg)
	{
	    msg->hwnd    = hwnd;
	    msg->message = WM_QUIT;
	    msg->wParam  = msgQueue->wExitCode;
	    msg->lParam  = 0;
	    break;
	}

	  /* Now find a WM_PAINT message */
	if ((msgQueue->status & QS_PAINT) && (mask & QS_PAINT))
	{
//		TRACE("paint");
	    msg->hwnd = WIN_FindWinToRepaint( hwnd , hQueue );
	    msg->message = WM_PAINT;
	    msg->wParam = 0;
	    msg->lParam = 0;
            if( msg->hwnd &&
              (!hwnd || msg->hwnd == hwnd || IsChild(hwnd,msg->hwnd)) )
              {
                WND* wndPtr = WIN_FindWndPtr(msg->hwnd);

	        /* FIXME: WM_PAINTICON should be sent sometimes */

                if( wndPtr->flags & WIN_INTERNAL_PAINT && !wndPtr->hrgnUpdate)
                  {
                    wndPtr->flags &= ~WIN_INTERNAL_PAINT;
                    QUEUE_DecPaintCount( hQueue );
                  }
                break;
              }
	}


	  /* Finally handle WM_TIMER messages */
	if ((msgQueue->status & QS_TIMER) && (mask & QS_TIMER))
	{
//@todo	    if (TIMER_CheckTimer( &nextExp, msg, hwnd, flags & PM_REMOVE ))
//@todo		break;  /* Got a timer msg */
	}
	else nextExp = -1;  /* No timeout needed */


        //Yield();      //UserYield
//	DirectedYield(0); // No multitrasking yet...
#if 0
	  /* Wait until something happens */
        if (peek)
        {
            if (!MSG_WaitXEvent( 0 )) return FALSE;  /* No pending event */
        }
        else  /* Wait for an event, then restart the loop */
            MSG_WaitXEvent( nextExp );
#endif
    }

      /* We got a message */
    return (msg->message != WM_QUIT);
}


/***********************************************************************
 *           MSG_InternalGetMessage
 *
 * GetMessage() function for internal use. Behave like GetMessage(),
 * but also call message filters and optionally send WM_ENTERIDLE messages.
 * 'hwnd' must be the handle of the dialog or menu window.
 * 'code' is the message filter value (MSGF_??? codes).
 */
BOOL MSG_InternalGetMessage( LPMSG msg, HWND hwnd, HWND hwndOwner, short code,
			     WORD flags, BOOL sendIdle ) 
{
    for (;;)
    {
	if (sendIdle)
	{
	    if (!MSG_PeekMessage( msg,
                                  0, 0, 0, flags, TRUE ))
	    {
		  /* No message present -> send ENTERIDLE and wait */
		SendMessage( hwndOwner, WM_ENTERIDLE, code, (LPARAM)hwnd );
		MSG_PeekMessage( msg,
                                 0, 0, 0, flags, FALSE );
	    }
	}
	else  /* Always wait for a message */
	    MSG_PeekMessage( msg,
                             0, 0, 0, flags, FALSE );

	if (!CallMsgFilter( msg, code ))
            return (msg->message != WM_QUIT);

	  /* Message filtered -> remove it from the queue */
	  /* if it's still there. */
	if (!(flags & PM_REMOVE))
	    MSG_PeekMessage( msg,
                             0, 0, 0, PM_REMOVE, TRUE );
    }
}


/***********************************************************************
 *           PeekMessage   (USER.109)
 */
BOOL WINAPI PeekMessage( LPMSG msg, HWND hwnd, UINT first, UINT last, UINT flags )
{
    return MSG_PeekMessage( msg, hwnd, first, last, flags, TRUE );
}


/***********************************************************************
 *           GetMessage   (USER.108)
 */
BOOL WINAPI GetMessage( LPMSG msg, HWND hwnd, UINT first, UINT last ) 
{
    MSG FAR * lpmsg = msg;

//	FUNCTION_START

    MSG_PeekMessage(lpmsg, hwnd, first, last, PM_REMOVE, FALSE);

//    TRACE("message %04x, hwnd %04x, filter(%04x - %04x)\n", lpmsg->message,
//		     				                 hwnd, first, last );
//@todo fix!!!    HOOK_CallHooks( WH_GETMESSAGE, HC_ACTION, 0, (LPARAM)msg );
    return (lpmsg->message != WM_QUIT);
}



/***********************************************************************
 *           PostMessage   (USER.110)
 */
BOOL WINAPI PostMessage( HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam )
{
    MSG 	msg;
    WND 	*wndPtr;

    msg.hwnd    = hwnd;
    msg.message = message;
    msg.wParam  = wParam;
    msg.lParam  = lParam;
    msg.time    = GetTickCount();
    msg.pt.x    = 0;
    msg.pt.y    = 0;

#ifdef CONFIG_IPC
    if (DDE_PostMessage(&msg))
       return TRUE;
#endif  /* CONFIG_IPC */
    
    if (hwnd == HWND_BROADCAST)
    {
        TRACE("PostMessage // HWND_BROADCAST !\n");
        for (wndPtr = WIN_GetDesktop()->child; wndPtr; wndPtr = wndPtr->next)
        {
            if (wndPtr->dwStyle & WS_POPUP || wndPtr->dwStyle & WS_CAPTION)
            {
                TRACE("BROADCAST Message to hWnd=%04x m=%04X w=%04X l=%08lX !\n",
                            wndPtr->hwndSelf, message, wParam, lParam);
                PostMessage( wndPtr->hwndSelf, message, wParam, lParam );
            }
        }
        TRACE("PostMessage // End of HWND_BROADCAST !\n");
        return TRUE;
    }

    wndPtr = WIN_FindWndPtr( hwnd );
    if (!wndPtr || !wndPtr->hmemTaskQ) return FALSE;

    return QUEUE_AddMsg( wndPtr->hmemTaskQ, &msg, 0 );
}

/***********************************************************************
 *           PostAppMessage   (USER.116)
 */
BOOL WINAPI PostAppMessage( HTASK hTask, UINT message, WPARAM wParam, LPARAM lParam )
{
    MSG 	msg;

    if (GetTaskQueue(hTask) == 0) return FALSE;
    msg.hwnd    = 0;
    msg.message = message;
    msg.wParam  = wParam;
    msg.lParam  = lParam;
    msg.time    = GetTickCount();
    msg.pt.x    = 0;
    msg.pt.y    = 0;

    return QUEUE_AddMsg( GetTaskQueue(hTask), &msg, 0 );
}


/***********************************************************************
 *           SendMessage   (USER.111)
 */
LRESULT WINAPI SendMessage( HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam )
{
    WND * wndPtr;
    LONG ret;
    struct
    {
	LPARAM lParam;
	WPARAM wParam;
	UINT wMsg;
	HWND hWnd;
    } msgstruct;


	msgstruct.lParam=lParam;
	msgstruct.wParam=wParam;
	msgstruct.wMsg=msg;
	msgstruct.hWnd=hwnd;

#ifdef CONFIG_IPC
    MSG DDE_msg = { hwnd, msg, wParam, lParam };
    if (DDE_SendMessage(&DDE_msg)) return TRUE;
#endif  /* CONFIG_IPC */

    if (hwnd == HWND_BROADCAST)
    {
        TRACE("SendMessage // HWND_BROADCAST !\n");
        for (wndPtr = WIN_GetDesktop()->child; wndPtr; wndPtr = wndPtr->next)
        {
            if (wndPtr->dwStyle & WS_POPUP || wndPtr->dwStyle & WS_CAPTION)
            {
                TRACE("BROADCAST Message to hWnd=%04x m=%04X w=%04lX l=%08lX !\n",
                            wndPtr->hwndSelf, msg, (DWORD)wParam, lParam);
                ret |= SendMessage( wndPtr->hwndSelf, msg, wParam, lParam );
	    }
        }
        TRACE("SendMessage // End of HWND_BROADCAST !\n");
        return TRUE;
    }

//    SPY_EnterMessage( SPY_SENDMESSAGE, hwnd, msg, wParam, lParam );
//    HOOK_CallHooks( WH_CALLWNDPROC, HC_ACTION, 1, (LPARAM)MAKE_SEGPTR(&msgstruct) );
    if (!(wndPtr = WIN_FindWndPtr( hwnd ))) 
    {
//        SPY_ExitMessage( SPY_RESULT_INVALIDHWND, hwnd, msg, 0 );
        return 0;
    }
    ret = CallWindowProc( (FARPROC)wndPtr->lpfnWndProc, msgstruct.hWnd, msgstruct.wMsg,
                          msgstruct.wParam, msgstruct.lParam );
//    SPY_ExitMessage( SPY_RESULT_OK, hwnd, msg, ret );
    return ret;
}


/***********************************************************************
 *           WaitMessage    (USER.112)
 */
void WINAPI WaitMessage( void )
{
    MSG msg;
    MESSAGEQUEUE *queue;
    LONG nextExp = -1;  /* Next timer expiration time */

#ifdef CONFIG_IPC
    DDE_GetRemoteMessage();
#endif  /* CONFIG_IPC */
    
    if (!(queue = (MESSAGEQUEUE FAR *)GlobalLock( GetTaskQueue(0) ))) return;
    if ((queue->wPostQMsg) || 
        (queue->status & (QS_SENDMESSAGE | QS_PAINT)) ||
        (queue->msgCount) || (QUEUE_GetSysQueue()->msgCount) )
        return;
    if ((queue->status & QS_TIMER) /* @todo && 
        TIMER_CheckTimer( &nextExp, &msg, 0, FALSE)*/)
        return;
    /* FIXME: (dde) must check DDE & X-events simultaneously */
//    MSG_WaitXEvent( nextExp );
}


/***********************************************************************
 *           TranslateMessage   (USER.113)
 *
 * This should call ToAscii but it is currently broken
 */

#define ASCII_CHAR_HACK 0x0800

BOOL WINAPI TranslateMessage(const MSG FAR *msg )
{
    UINT message = msg->message;
    /* BYTE wparam[2]; */
    
    if ((message == WM_KEYDOWN) || (message == WM_KEYUP) ||
	(message == WM_SYSKEYDOWN) || (message == WM_SYSKEYUP))
    {
	TRACE("Translating key %04x, scancode %04x\n", msg->wParam, 
							      HIWORD(msg->lParam) );

	if( HIWORD(msg->lParam) & ASCII_CHAR_HACK )

	/*  if( ToAscii( msg->wParam, HIWORD(msg->lParam), (LPSTR)&KeyStateTable,
				      wparam, 0 ) ) 
         */
	      {
     		message += 2 - (message & 0x0001); 

	        PostMessage( msg->hwnd, message, lastEventChar, msg->lParam );

	        return TRUE;
	      }
    }
    return FALSE;
}


/***********************************************************************
 *           DispatchMessage   (USER.114)
 */
LONG WINAPI DispatchMessage( const MSG* msg )
{
    WND * wndPtr;
    LONG retval;
    int painting;
    
//    SPY_EnterMessage( SPY_DISPATCHMESSAGE, msg->hwnd, msg->message,
//                      msg->wParam, msg->lParam );

//FUNCTION_START
      /* Process timer messages */
    if ((msg->message == WM_TIMER) /*|| (msg->message == WM_SYSTIMER)*/)
    {
	if (msg->lParam)
        {
#ifndef WINELIB
            HINSTANCE ds = msg->hwnd ? WIN_GetWindowInstance( msg->hwnd )
                                     : (HINSTANCE)GetDS()/*CURRENT_DS*/;
#endif
/*            HOOK_CallHooks( WH_CALLWNDPROC, HC_ACTION, 0, FIXME ); */
	    return CallWindowProc( (FARPROC)msg->lParam, /*ds,*/ msg->hwnd,
                                msg->message, msg->wParam, GetTickCount() );
        }
    }

    if (!msg->hwnd) return 0;
    if (!(wndPtr = WIN_FindWndPtr( msg->hwnd ))) return 0;
    if (!wndPtr->lpfnWndProc) return 0;
    painting = (msg->message == WM_PAINT);
    if (painting) wndPtr->flags |= WIN_NEEDS_BEGINPAINT;
/*    HOOK_CallHooks( WH_CALLWNDPROC, HC_ACTION, 0, FIXME ); */
    retval = CallWindowProc( (FARPROC)wndPtr->lpfnWndProc, msg->hwnd, msg->message,
			     msg->wParam, msg->lParam );
    if (painting && (wndPtr = WIN_FindWndPtr( msg->hwnd )) &&
        (wndPtr->flags & WIN_NEEDS_BEGINPAINT) && wndPtr->hrgnUpdate)
    {
	TRACE("BeginPaint not called on WM_PAINT for hwnd %04x!\n", 
		msg->hwnd);
	wndPtr->flags &= ~WIN_NEEDS_BEGINPAINT;
        /* Validate the update region to avoid infinite WM_PAINT loop */
        ValidateRect( msg->hwnd, NULL );
    }
    return retval;
}


/***********************************************************************
 *           RegisterWindowMessage   (USER.118)
 */
UINT WINAPI RegisterWindowMessage( LPCSTR str )
{
    TRACE("RegisterWindowMessage: %08lx\n", (DWORD)str );
    return GlobalAddAtom( str );
}

#if 0
/***********************************************************************
 *           GetTickCount    (USER.13) (KERNEL32.299)
 */
DWORD GetTickCount(void)
{
    struct timeval t;
    gettimeofday( &t, NULL );
    return ((t.tv_sec * 1000) + (t.tv_usec / 1000)) - MSG_WineStartTicks;
}


/***********************************************************************
 *           GetCurrentTime    (USER.15)
 *
 * (effectively identical to GetTickCount)
 */
DWORD GetCurrentTime(void)
{
    return GetTickCount();
}
#endif

/***********************************************************************
 *			InSendMessage	(USER.192
 *
 * According to the book, this should return true iff the current message
 * was send from another application. In that case, the application should
 * invoke ReplyMessage before calling message relevant API.
 * Currently, Wine will always return FALSE, as there is no other app.
 */
BOOL WINAPI InSendMessage()
{
	return FALSE;
}
