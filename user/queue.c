/*
 * Message queues related functions
 *
 * Copyright 1993, 1994 Alexandre Julliard
 */

//#include <stdlib.h>
//#include "module.h"
#include "user.h"
#include "queue.h"
//#include "win.h"
//#include "stddebug.h"
//#include "debug.h"

#define MAX_QUEUE_SIZE   120  /* Max. size of a message queue */

static HQUEUE hFirstQueue = 0;
static HQUEUE hmemSysMsgQueue = 0;
static MESSAGEQUEUE FAR *sysMsgQueue = NULL;

/***********************************************************************
 *	     QUEUE_DumpQueue
 */
void QUEUE_DumpQueue( HQUEUE hQueue )
{
    MESSAGEQUEUE FAR *pq; 

    if (!(pq = (MESSAGEQUEUE FAR *) GlobalLock( hQueue )) ||
        GlobalSize(hQueue) < sizeof(MESSAGEQUEUE) + pq->queueSize*sizeof(QMSG))
    {
        TRACE("%04x is not a queue handle", hQueue );
        return;
    }

    TRACE(
             "next: %12.4x  Intertask SendMessage:\n\r"
             "hTask: %11.4x  ----------------------\n\r"
             "msgSize: %9.4x  hWnd: %10.4x\n\r"
             "msgCount: %8.4d  msg: %11.4x\n\r"
             "msgNext: %9.4x  wParam: %8.4x\n\r"
             "msgFree: %9.4x  lParam: %8.8x\n\r"
             "qSize: %11.4x  lRet: %10.8x\n\r"
             "wWinVer: %9.4x  ISMH: %10.4x\n\r"
             "paints: %10.4x  hSendTask: %5.4x\n\r"
             "timers: %10.4x  hPrevSend: %5.4x\n\r"
             "wakeBits: %8.4x\n\r"
             "wakeMask: %8.4x\n\r"
             "hCurHook: %8.4x\n\r",
             pq->next, pq->hTask, pq->msgSize, pq->hWnd, 
             pq->msgCount, pq->msg, pq->nextMessage, pq->wParam,
             pq->nextFreeMessage, (unsigned)pq->lParam, pq->queueSize,
             (unsigned)pq->SendMessageReturn, pq->wWinVersion, pq->InSendMessageHandle,
             pq->wPaintCount, pq->hSendingTask, pq->wTimerCount,
             pq->hPrevSendingTask, pq->status, pq->wakeMask, pq->hCurHook);
}


/***********************************************************************
 *	     QUEUE_WalkQueues
 */
void QUEUE_WalkQueues(void)
{
    HQUEUE hQueue = hFirstQueue;

    TRACE("Queue Size Msgs Task\n" );
    while (hQueue)
    {
        MESSAGEQUEUE FAR *queue = (MESSAGEQUEUE FAR *)GlobalLock( hQueue );
        if (!queue)
        {
            TRACE("*** Bad queue handle %04x\n", hQueue );
            return;
        }
        TRACE("%04x %5d %4d %04x %S\n",
                 hQueue, queue->msgSize, queue->msgCount, queue->hTask,
                 ""/*MODULE_GetModuleName( GetExePtr(queue->hTask) )*/ );
        hQueue = queue->next;
    }
}


/***********************************************************************
 *           QUEUE_CreateMsgQueue
 *
 * Creates a message queue. Doesn't link it into queue list!
 */
static HQUEUE QUEUE_CreateMsgQueue( int size )
{
    HQUEUE hQueue;
    MESSAGEQUEUE FAR * msgQueue;
    int queueSize;

    queueSize = sizeof(MESSAGEQUEUE) + size * sizeof(QMSG);
    if (!(hQueue = GlobalAlloc( GMEM_FIXED | GMEM_ZEROINIT, queueSize )))
        return 0;
    msgQueue = (MESSAGEQUEUE FAR *) GlobalLock( hQueue );
    msgQueue->msgSize = sizeof(QMSG);
    msgQueue->queueSize = size;
    msgQueue->wWinVersion = 0;  /* FIXME? */
    GlobalUnlock( hQueue );
    return hQueue;
}


/***********************************************************************
 *	     QUEUE_DeleteMsgQueue
 *
 * Unlinks and deletes a message queue.
 */
BOOL QUEUE_DeleteMsgQueue( HQUEUE hQueue )
{
    MESSAGEQUEUE FAR * msgQueue = (MESSAGEQUEUE FAR *)GlobalLock(hQueue);
    HQUEUE *pPrev;

    if (!hQueue || !msgQueue)
    {
	TRACE("DeleteMsgQueue: invalid argument.\n");
	return 0;
    }

    pPrev = &hFirstQueue;
    while (*pPrev && (*pPrev != hQueue))
    {
        MESSAGEQUEUE FAR *msgQ = (MESSAGEQUEUE FAR *)GlobalLock(*pPrev);
        pPrev = &msgQ->next;
    }
    if (*pPrev) *pPrev = msgQueue->next;
    GlobalFree( hQueue );
    return 1;
}


/***********************************************************************
 *           QUEUE_CreateSysMsgQueue
 *
 * Create the system message queue, and set the double-click speed.
 * Must be called only once.
 */
BOOL QUEUE_CreateSysMsgQueue( int size )
{
    if (size > MAX_QUEUE_SIZE) size = MAX_QUEUE_SIZE;
    else if (size <= 0) size = 1;
    if (!(hmemSysMsgQueue = QUEUE_CreateMsgQueue( size ))) return FALSE;
    sysMsgQueue = (MESSAGEQUEUE FAR *) GlobalLock( hmemSysMsgQueue );
    return TRUE;
}


/***********************************************************************
 *           QUEUE_GetSysQueue
 */
MESSAGEQUEUE FAR *QUEUE_GetSysQueue(void)
{
    return sysMsgQueue;
}


/***********************************************************************
 *           QUEUE_AddMsg
 *
 * Add a message to the queue. Return FALSE if queue is full.
 */
BOOL QUEUE_AddMsg( HQUEUE hQueue, MSG * msg, DWORD extraInfo )
{
    int pos;
    MESSAGEQUEUE FAR *msgQueue;

    if (!(msgQueue = (MESSAGEQUEUE FAR *)GlobalLock( hQueue ))) return FALSE;
    pos = msgQueue->nextFreeMessage;

      /* Check if queue is full */
    if ((pos == msgQueue->nextMessage) && (msgQueue->msgCount > 0))
    {
        TRACE("MSG_AddMsg // queue is full !\n");
        return FALSE;
    }

      /* Store message */
    msgQueue->messages[pos].msg = *msg;
    msgQueue->messages[pos].extraInfo = extraInfo;
    if (pos < msgQueue->queueSize-1) pos++;
    else pos = 0;
    msgQueue->nextFreeMessage = pos;
    msgQueue->msgCount++;
    msgQueue->status |= QS_POSTMESSAGE;
    msgQueue->tempStatus |= QS_POSTMESSAGE;
    return TRUE;
}


/***********************************************************************
 *           QUEUE_FindMsg
 *
 * Find a message matching the given parameters. Return -1 if none available.
 */
int QUEUE_FindMsg( MESSAGEQUEUE FAR * msgQueue, HWND hwnd, int first, int last )
{
    int i, pos = msgQueue->nextMessage;

    TRACE("MSG_FindMsg: hwnd=%04x\n\n", hwnd );

    if (!msgQueue->msgCount) return -1;
    if (!hwnd && !first && !last) return pos;
        
    for (i = 0; i < msgQueue->msgCount; i++)
    {
	MSG * msg = &msgQueue->messages[pos].msg;

	if (!hwnd || (msg->hwnd == hwnd))
	{
	    if (!first && !last) return pos;
	    if ((msg->message >= first) && (msg->message <= last)) return pos;
	}
	if (pos < msgQueue->queueSize-1) pos++;
	else pos = 0;
    }
    return -1;
}


/***********************************************************************
 *           QUEUE_RemoveMsg
 *
 * Remove a message from the queue (pos must be a valid position).
 */
void QUEUE_RemoveMsg( MESSAGEQUEUE FAR * msgQueue, int pos )
{
    if (pos >= msgQueue->nextMessage)
    {
	for ( ; pos > msgQueue->nextMessage; pos--)
	    msgQueue->messages[pos] = msgQueue->messages[pos-1];
	msgQueue->nextMessage++;
	if (msgQueue->nextMessage >= msgQueue->queueSize)
	    msgQueue->nextMessage = 0;
    }
    else
    {
	for ( ; pos < msgQueue->nextFreeMessage; pos++)
	    msgQueue->messages[pos] = msgQueue->messages[pos+1];
	if (msgQueue->nextFreeMessage) msgQueue->nextFreeMessage--;
	else msgQueue->nextFreeMessage = msgQueue->queueSize-1;
    }
    msgQueue->msgCount--;
    if (!msgQueue->msgCount) msgQueue->status &= ~QS_POSTMESSAGE;
    msgQueue->tempStatus = 0;
}


/***********************************************************************
 *           hardware_event
 *
 * Add an event to the system message queue.
 */
void hardware_event( WORD message, WORD wParam, LONG lParam,
		     int xPos, int yPos, DWORD time, DWORD extraInfo )
{
    MSG *msg;
    int pos;
  
    if (!sysMsgQueue) return;
    pos = sysMsgQueue->nextFreeMessage;

      /* Merge with previous event if possible */

    if ((message == WM_MOUSEMOVE) && sysMsgQueue->msgCount)
    {
        if (pos > 0) pos--;
        else pos = sysMsgQueue->queueSize - 1;
	msg = &sysMsgQueue->messages[pos].msg;
	if ((msg->message == message) && (msg->wParam == wParam))
            sysMsgQueue->msgCount--;  /* Merge events */
        else
            pos = sysMsgQueue->nextFreeMessage;  /* Don't merge */
    }

      /* Check if queue is full */

    if ((pos == sysMsgQueue->nextMessage) && sysMsgQueue->msgCount)
    {
        /* Queue is full, beep (but not on every mouse motion...) */
//        if (message != WM_MOUSEMOVE) MessageBeep(0);
TRACE("FULL!!");
        return;
    }

      /* Store message */

    msg = &sysMsgQueue->messages[pos].msg;
    msg->hwnd    = 0;
    msg->message = message;
    msg->wParam  = wParam;
    msg->lParam  = lParam;
    msg->time    = time;
    msg->pt.x    = xPos & 0xffff;
    msg->pt.y    = yPos & 0xffff;
    sysMsgQueue->messages[pos].extraInfo = extraInfo;
    if (pos < sysMsgQueue->queueSize - 1) pos++;
    else pos = 0;
    sysMsgQueue->nextFreeMessage = pos;
    sysMsgQueue->msgCount++;

    QUEUE_DumpQueue(hmemSysMsgQueue);

}

		    
/***********************************************************************
 *	     QUEUE_GetQueueTask
 */
HTASK QUEUE_GetQueueTask( HQUEUE hQueue )
{
    MESSAGEQUEUE FAR *queue = (MESSAGEQUEUE FAR *)GlobalLock( hQueue );
    return (queue) ? queue->hTask : 0 ;
}


/***********************************************************************
 *           QUEUE_IncPaintCount
 */
void QUEUE_IncPaintCount( HQUEUE hQueue )
{
    MESSAGEQUEUE FAR *queue;

    if (!(queue = (MESSAGEQUEUE FAR *)GlobalLock( hQueue ))) return;
    queue->wPaintCount++;
    queue->status |= QS_PAINT;
    queue->tempStatus |= QS_PAINT;    
}


/***********************************************************************
 *           QUEUE_DecPaintCount
 */
void QUEUE_DecPaintCount( HQUEUE hQueue )
{
    MESSAGEQUEUE FAR *queue;

    if (!(queue = (MESSAGEQUEUE FAR *)GlobalLock( hQueue ))) return;
    queue->wPaintCount--;
    if (!queue->wPaintCount) queue->status &= ~QS_PAINT;
}


/***********************************************************************
 *           QUEUE_IncTimerCount
 */
void QUEUE_IncTimerCount( HQUEUE hQueue )
{
    MESSAGEQUEUE FAR *queue;

    if (!(queue = (MESSAGEQUEUE FAR *)GlobalLock( hQueue ))) return;
    queue->wTimerCount++;
    queue->status |= QS_TIMER;
    queue->tempStatus |= QS_TIMER;
}


/***********************************************************************
 *           QUEUE_DecTimerCount
 */
void QUEUE_DecTimerCount( HQUEUE hQueue )
{
    MESSAGEQUEUE FAR *queue;

    if (!(queue = (MESSAGEQUEUE FAR *)GlobalLock( hQueue ))) return;
    queue->wTimerCount--;
    if (!queue->wTimerCount) queue->status &= ~QS_TIMER;
}


/***********************************************************************
 *           PostQuitMessage   (USER.6)
 */
void WINAPI PostQuitMessage( int exitCode )
{
    MESSAGEQUEUE FAR *queue;

    if (!(queue = (MESSAGEQUEUE FAR *)GlobalLock( GetTaskQueue(0) ))) return;
    queue->wPostQMsg = TRUE;
    queue->wExitCode = (WORD)exitCode;
}

#if 0

/***********************************************************************
 *           GetWindowTask   (USER.224)
 */
HTASK GetWindowTask( HWND hwnd )
{
    WND *wndPtr = WIN_FindWndPtr( hwnd );

    if (!wndPtr) return 0;
    return QUEUE_GetQueueTask( wndPtr->hmemTaskQ );
}

#endif

/***********************************************************************
 *           SetMessageQueue   (USER.266)
 */
BOOL WINAPI SetMessageQueue( int size )
{
    HQUEUE hQueue, hNewQueue;
    MESSAGEQUEUE FAR *queuePtr;

    if ((size > MAX_QUEUE_SIZE) || (size <= 0)) return TRUE;

    if( !(hNewQueue = QUEUE_CreateMsgQueue( size ))) 
    {
	TRACE("SetMessageQueue: failed!\n");
	return FALSE;
    }

    /* Free the old message queue */
    if ((hQueue = GetTaskQueue(0)) != 0) QUEUE_DeleteMsgQueue( hQueue );

    /* Link new queue into list */
    queuePtr = (MESSAGEQUEUE FAR *)GlobalLock( hNewQueue );
    queuePtr->hTask = GetCurrentTask();
    queuePtr->next  = hFirstQueue;
    hFirstQueue = hNewQueue;

    SetTaskQueue( 0, hNewQueue );
    return TRUE;
}


/***********************************************************************
 *           GetQueueStatus   (USER.334)
 */
DWORD WINAPI GetQueueStatus( UINT flags )
{
    MESSAGEQUEUE FAR *queue;
    DWORD ret;

    if (!(queue = (MESSAGEQUEUE FAR *)GlobalLock( GetTaskQueue(0) ))) return 0;
    ret = MAKELONG( queue->tempStatus, queue->status );
    queue->tempStatus = 0;
    return ret & MAKELONG( flags, flags );
}


/***********************************************************************
 *           GetInputState   (USER.335)
 */
BOOL WINAPI GetInputState()
{
    MESSAGEQUEUE FAR *queue;

    if (!(queue = (MESSAGEQUEUE FAR *)GlobalLock( GetTaskQueue(0) ))) return FALSE;
    return queue->status & (QS_KEY | QS_MOUSEBUTTON);
}


/***********************************************************************
 *           GetMessagePos   (USER.119)
 */
DWORD WINAPI GetMessagePos(void)
{
    MESSAGEQUEUE FAR *queue;

    if (!(queue = (MESSAGEQUEUE FAR *)GlobalLock( GetTaskQueue(0) ))) return 0;
    return queue->GetMessagePosVal;
}


/***********************************************************************
 *           GetMessageTime   (USER.120)
 */
LONG WINAPI GetMessageTime(void)
{
    MESSAGEQUEUE FAR *queue;

    if (!(queue = (MESSAGEQUEUE FAR *)GlobalLock( GetTaskQueue(0) ))) return 0;
    return queue->GetMessageTimeVal;
}


/***********************************************************************
 *           GetMessageExtraInfo   (USER.288)
 */
LONG WINAPI GetMessageExtraInfo(void)
{
    MESSAGEQUEUE FAR *queue;

    if (!(queue = (MESSAGEQUEUE FAR *)GlobalLock( GetTaskQueue(0) ))) return 0;
    return queue->GetMessageExtraInfoVal;
}
