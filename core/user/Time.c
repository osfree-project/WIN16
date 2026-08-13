/*
 *
 * Timer functions
 *
 * Copyright 1993 Alexandre Julliard
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, see
 * <https://www.gnu.org/licenses/>.
 *
 */

#include <user.h>

DWORD WINAPI GetSystemMSecCount(void);

#pragma code_seg( "FIXED_TEXT" );

DWORD WINAPI GetCurrentTime()
{
	FUNCTION_START
    return GetSystemMSecCount();
}

DWORD WINAPI GetTickCount()
{
//	FUNCTION_START
    return GetSystemMSecCount();
}

typedef struct tagTIMER
{
    HWND             hwnd;
    HQUEUE	     hq;
    WORD             msg;  /* WM_TIMER or WM_SYSTIMER */
    WORD             id;
    WORD             timeout;
    struct tagTIMER *next;
    DWORD            expires;
    FARPROC          proc;
} TIMER;

#define NB_TIMERS            34
#define NB_RESERVED_TIMERS    2  /* for SetSystemTimer */

static TIMER TimersArray[NB_TIMERS];

static TIMER * pNextTimer = NULL;  /* Next timer to expire */

  /* Duration from 'time' until expiration of the timer */
#define EXPIRE_TIME(pTimer,time) \
          (((pTimer)->expires <= (time)) ? 0 : (pTimer)->expires - (time))


/***********************************************************************
 *           TIMER_InsertTimer
 *
 * Insert the timer at its place in the chain.
 */
static void TIMER_InsertTimer( TIMER * pTimer )
{
    if (!pNextTimer || (pTimer->expires < pNextTimer->expires))
    {
	pTimer->next = pNextTimer;
	pNextTimer = pTimer;
    }
    else
    {
        TIMER * ptr = pNextTimer;	
	while (ptr->next && (pTimer->expires >= ptr->next->expires))
	    ptr = ptr->next;
	pTimer->next = ptr->next;
	ptr->next = pTimer;
    }
}


/***********************************************************************
 *           TIMER_RemoveTimer
 *
 * Remove the timer from the chain.
 */
static void TIMER_RemoveTimer( TIMER * pTimer )
{
    if (pTimer == pNextTimer) pNextTimer = pTimer->next;
    else
    {
	TIMER * ptr = pNextTimer;
	while (ptr && (ptr->next != pTimer)) ptr = ptr->next;
	if (ptr) ptr->next = pTimer->next;
    }
    pTimer->next = NULL;
}

/***********************************************************************
 *           TIMER_SwitchQueue
 */
void TIMER_SwitchQueue(HQUEUE old, HQUEUE new)
{
 TIMER*         pT = pNextTimer;

 while(pT)
  {
   if( pT->hq == old ) pT->hq = new;
   pT = pT->next;
  }

}

/***********************************************************************
 *           TIMER_NukeTimers
 *
 * Trash all timers that are bound to the hwnd or hq
 */
void TIMER_NukeTimers(HWND hwnd, HQUEUE hq)
{
 HQUEUE		hQToUpdate = ( hwnd ) ? GetTaskQueue( GetWindowTask( hwnd ) )
				      : hq;
 TIMER*         pT = pNextTimer;
 TIMER*         pTnext;

 if( !pT ) return;

 while( (hwnd && pT->hwnd == hwnd) ||
        (hq && pT->hq == hq) )
      {
	 QUEUE_DecTimerCount( hQToUpdate );
         if( !(pT = pNextTimer = pNextTimer->next) )
             return;
      }

 /* pT points to the "good" timer */

 while( (pTnext = pT->next) )
    {
      while( (hwnd && pTnext->hwnd == hwnd) ||
             (hq && pTnext->hq == hq) )
	   {
	      QUEUE_DecTimerCount( hQToUpdate );
              if( !(pT->next = pTnext->next) )
                  return;
	   }

      pT = pT->next;
    }
}

/***********************************************************************
 *           TIMER_RestartTimers
 *
 * Restart an expired timer.
 */
static void TIMER_RestartTimer( TIMER * pTimer, DWORD curTime )
{
    TIMER_RemoveTimer( pTimer );
    pTimer->expires = curTime + pTimer->timeout;
    TIMER_InsertTimer( pTimer );
}

			       
/***********************************************************************
 *           TIMER_CheckTimer
 *
 * Check whether a timer has expired, and create a message if necessary.
 * Otherwise, return time until next timer expiration in 'next'.
 * If 'hwnd' is not NULL, only consider timers for this window.
 * If 'remove' is TRUE, remove all expired timers up to the returned one.
 */
BOOL TIMER_CheckTimer( LONG *next, MSG *msg, HWND hwnd, BOOL remove )
{
    TIMER * pTimer = pNextTimer;
    DWORD curTime = GetTickCount();

    if (hwnd)  /* Find first timer for this window */
	while (pTimer && (pTimer->hwnd != hwnd)) pTimer = pTimer->next;

    if (!pTimer) *next = -1;
    else *next = EXPIRE_TIME( pTimer, curTime );
    if (*next != 0) return FALSE;  /* No timer expired */

    if (remove)	/* Restart all timers before pTimer, and then pTimer itself */
    {
	while (pNextTimer != pTimer) TIMER_RestartTimer( pNextTimer, curTime );
	TIMER_RestartTimer( pTimer, curTime );
    }

//    dprintf_timer(stddeb, "Timer expired: %p, %04x, %04x, %04x, %08lx\n", 
//		  pTimer, pTimer->hwnd, pTimer->msg, pTimer->id, (DWORD)pTimer->proc);
      /* Build the message */
    msg->hwnd    = pTimer->hwnd;
    msg->message = pTimer->msg;
    msg->wParam  = pTimer->id;
    msg->lParam  = (LONG)pTimer->proc;
    msg->time    = curTime;
    return TRUE;
}


/***********************************************************************
 *           TIMER_SetTimer
 */
static WORD TIMER_SetTimer( HWND hwnd, WORD id, WORD timeout,
			    FARPROC proc, BOOL sys )
{
    int i;
    TIMER * pTimer;

    if (!timeout) return 0;
/*    if (!hwnd && !proc) return 0; */

      /* Check if there's already a timer with the same hwnd and id */

    for (i = 0, pTimer = TimersArray; i < NB_TIMERS; i++, pTimer++)
        if ((pTimer->hwnd == hwnd) && (pTimer->id == id) &&
            (pTimer->timeout != 0))
        {
              /* Got one: set new values and return */
            pTimer->timeout = timeout;
            pTimer->expires = GetTickCount() + timeout;
            pTimer->proc    = proc;
            TIMER_RemoveTimer( pTimer );
            TIMER_InsertTimer( pTimer );
            return id;
        }

      /* Find a free timer */
    
    for (i = 0, pTimer = TimersArray; i < NB_TIMERS; i++, pTimer++)
	if (!pTimer->timeout) break;

    if (i >= NB_TIMERS) return 0;
    if (!sys && (i >= NB_TIMERS-NB_RESERVED_TIMERS)) return 0;
    if (!hwnd) id = i + 1;
    
      /* Add the timer */

    pTimer->hwnd    = hwnd;
    pTimer->hq	    = (hwnd) ? GetTaskQueue( GetWindowTask( hwnd ) )
			     : GetTaskQueue( 0 );
    pTimer->msg     = sys ? WM_SYSTIMER : WM_TIMER;
    pTimer->id      = id;
    pTimer->timeout = timeout;
    pTimer->expires = GetTickCount() + timeout;
    pTimer->proc    = proc;
//    dprintf_timer(stddeb, "Timer added: %p, %04x, %04x, %04x, %08lx\n", 
//		  pTimer, pTimer->hwnd, pTimer->msg, pTimer->id, (DWORD)pTimer->proc);
    TIMER_InsertTimer( pTimer );
    QUEUE_IncTimerCount( pTimer->hq );
    if (!id)
	return TRUE;
    else
	return id;
}


/***********************************************************************
 *           TIMER_KillTimer
 */
static BOOL TIMER_KillTimer( HWND hwnd, WORD id, BOOL sys )
{
    int i;
    TIMER * pTimer;
    HQUEUE  hq;
    
      /* Find the timer */
    
    for (i = 0, pTimer = TimersArray; i < NB_TIMERS; i++, pTimer++)
	if ((pTimer->hwnd == hwnd) && (pTimer->id == id) &&
	    (pTimer->timeout != 0)) break;
    if (i >= NB_TIMERS) return FALSE;
    if (!sys && (i >= NB_TIMERS-NB_RESERVED_TIMERS)) return FALSE;
    if (!sys && (pTimer->msg != WM_TIMER)) return FALSE;
    else if (sys && (pTimer->msg != WM_SYSTIMER)) return FALSE;    

      /* Delete the timer */

    hq = pTimer->hq;

    pTimer->hwnd    = 0;
    pTimer->msg     = 0;
    pTimer->id      = 0;
    pTimer->timeout = 0;
    pTimer->proc    = 0;
    TIMER_RemoveTimer( pTimer );
    QUEUE_DecTimerCount( hq );
    return TRUE;
}

/***********************************************************************
 *           SetTimer   (USER.10)
 */
UINT WINAPI SetTimer( HWND hwnd, UINT id, UINT timeout, FARPROC proc )
{
	UINT retVal;
	FUNCTION_START
	retVal=TIMER_SetTimer( hwnd, id, timeout, proc, FALSE );
	FUNCTION_END
	return retVal;
}

#define SetSystemTimer BEAR11

/***********************************************************************
 *		SetSystemTimer (USER.11)
 */
UINT WINAPI SetSystemTimer( HWND hwnd, UINT id, UINT timeout, TIMERPROC proc )
{
	FUNCTION_START
    //TIMERPROC proc32 = (TIMERPROC)WINPROC_AllocProc16( (WNDPROC16)proc );
    //return SetTimer( WIN_Handle32(hwnd), (UINT_PTR)id | SYSTEM_TIMER_FLAG, timeout, proc32 );
    return TIMER_SetTimer( hwnd, id, timeout, proc, TRUE );
//	return 0;
}

/***********************************************************************
 *           KillTimer   (USER.12)
 */
BOOL WINAPI KillTimer( HWND hwnd, UINT id )
{
	BOOL retVal;
	FUNCTION_START
//    dprintf_timer(stddeb, "KillTimer: %04x %d\n", hwnd, id );
	retVal=TIMER_KillTimer( hwnd, id, FALSE );;
	FUNCTION_END
	return retVal;
}

#define KillSystemTimer BEAR182

/**************************************************************************
 *              KillSystemTimer   (USER.182)
 */
BOOL WINAPI KillSystemTimer( HWND hwnd, UINT id )
{
	FUNCTION_START
    return TIMER_KillTimer( hwnd, id, TRUE );
//    return FALSE;//KillTimer( WIN_Handle32(hwnd), (UINT_PTR)id | SYSTEM_TIMER_FLAG );
}

#pragma code_seg();
