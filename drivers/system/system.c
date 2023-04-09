/*
 * SYSTEM DLL routines
 *
 * Copyright 1996 Alexandre Julliard
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
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301, USA
 */

//#include <stdarg.h>

//#define NONAMELESSUNION

#include <windows.h>

typedef struct
{
    FARPROC         callback;
    long             rate;
    long             ticks;
} SYSTEM_TIMER;

#define NB_SYS_TIMERS   8
#define SYS_TIMER_RATE  54925

static DWORD clockTick = 0;
static SYSTEM_TIMER SYS_Timers[NB_SYS_TIMERS];
static int SYS_NbTimers = 0;
static HANDLE SYS_timer;
static HANDLE SYS_thread;
static BOOL SYS_timers_disabled;
void interrupt (*PrevTimerIntHandler)(void);

extern  void    disable( void );
#pragma aux     disable = 0xfa;

extern  void    enable( void );
#pragma aux     enable = 0xfb;

extern  void (__interrupt far *getvect( int ax ))();
#pragma aux getvect = \
            "mov ah,35h"    \
            "int 21h"       \
        __parm      [__ax] \
        __value     [__es __bx]

extern  void setvect( int, void (__interrupt far *)());
#pragma aux setvect = \
            "push ds"           \
            "mov ds,cx"         \
            "mov ah,25h"        \
            "int 21h"           \
            "pop ds"            \
        __parm __caller [__ax] [__cx __dx]

static void interrupt SYSTEM_TimerTick(void)
{
    int i;


    if (!SYS_timers_disabled) 
    {
      clockTick=clockTick+SYS_TIMER_RATE/1000;
      for (i = 0; i < NB_SYS_TIMERS; i++)
      {
          if (!SYS_Timers[i].callback) continue;
          if ((SYS_Timers[i].ticks -= SYS_TIMER_RATE) <= 0)
          {
              FARPROC proc = SYS_Timers[i].callback;
  
              SYS_Timers[i].ticks += SYS_Timers[i].rate;
  
          }
      }
    }
    (*PrevTimerIntHandler)(); /* Call the call the original interrupt handler */
}


/**********************************************************************
 *           SYSTEM_StartTicks
 *
 * Start the system tick timer.
 */
static void SYSTEM_StartTicks(void)
{
    disable();
    PrevTimerIntHandler = getvect(8);
    setvect(8, SYSTEM_TimerTick);
    enable();
}


/**********************************************************************
 *           SYSTEM_StopTicks
 *
 * Stop the system tick timer.
 */
static void SYSTEM_StopTicks(void)
{
    disable();
    setvect(8, PrevTimerIntHandler);
    enable();
}

/***********************************************************************
 *           InquireSystem   (SYSTEM.1)
 *
 * Note: the function always takes 2 WORD arguments, contrary to what
 *       "Undocumented Windows" says.
  */
DWORD WINAPI InquireSystem( WORD code, WORD arg )
{
    WORD drivetype;
    char root[3];

    switch(code)
    {
    case 0:  /* Get timer resolution */
        return SYS_TIMER_RATE;

    case 1:  /* Get drive type */
        root[0] = 'A' + arg;
        root[1] = ':';
        root[2] = 0;
//        drivetype = GetDriveType( root );
//        if (drivetype == DRIVE_CDROM) drivetype = DRIVE_REMOTE;
//        else if (drivetype == DRIVE_NO_ROOT_DIR) drivetype = DRIVE_UNKNOWN;
        return MAKELONG( drivetype, drivetype );

    case 2:  /* Enable one-drive logic */
//        FIXME("Case %d: set single-drive %d not supported\n", code, arg );
        return 0;
    }
//    WARN("Unknown code %d\n", code );
    return 0;
}


/***********************************************************************
 *           CreateSystemTimer   (SYSTEM.2)
 */
WORD WINAPI CreateSystemTimer( WORD rate, FARPROC proc )
{
    int i;
    for (i = 0; i < NB_SYS_TIMERS; i++)
        if (!SYS_Timers[i].callback)  /* Found one */
        {
            SYS_Timers[i].rate = (UINT)rate * 1000;
            if (SYS_Timers[i].rate < SYS_TIMER_RATE)
                SYS_Timers[i].rate = SYS_TIMER_RATE;
            SYS_Timers[i].ticks = SYS_Timers[i].rate;
            SYS_Timers[i].callback = proc;
            if (++SYS_NbTimers == 1) SYSTEM_StartTicks();
            return i + 1;  /* 0 means error */
        }
    return 0;
}


/***********************************************************************
 *           KillSystemTimer   (SYSTEM.3)
 *
 * Note: do not confuse this function with USER.182
 */
WORD WINAPI KillSystemTimer( WORD timer )
{
    if ( !timer || timer > NB_SYS_TIMERS || !SYS_Timers[timer-1].callback )
        return timer;  /* Error */
    SYS_Timers[timer-1].callback = 0;
    if (!--SYS_NbTimers) SYSTEM_StopTicks();
    return 0;
}


/***********************************************************************
 *           EnableSystemTimers   (SYSTEM.4)
 */
void WINAPI EnableSystemTimers(void)
{
    SYS_timers_disabled = FALSE;
}


/***********************************************************************
 *           DisableSystemTimers   (SYSTEM.5)
 */
void WINAPI DisableSystemTimers(void)
{
    SYS_timers_disabled = TRUE;
}


/***********************************************************************
 *           GetSystemMSecCount (SYSTEM.6)
 */
DWORD WINAPI GetSystemMSecCount(void)
{
    return clockTick;
}


/***********************************************************************
 *           Get80x87SaveSize   (SYSTEM.7)
 */
WORD WINAPI Get80x87SaveSize(void)
{
    return 94;
}


/***********************************************************************
 *           Save80x87State   (SYSTEM.8)
 */
void WINAPI Save80x87State( LPSTR pptr )
{
  _asm {
	les	bx,pptr
	fsave	es:[bx]
  }
}


/***********************************************************************
 *           Restore80x87State   (SYSTEM.9)
 */
void WINAPI Restore80x87State( LPCSTR pptr )
{
   _asm {
	les	bx,pptr
	frstor	es:[bx]
   }
}


/***********************************************************************
 *           A20_Proc  (SYSTEM.20)
 */
void WINAPI A20_Proc( WORD unused )
{
    /* this is also a NOP in Windows */
}

BOOL WINAPI LibMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved)
{
	return TRUE;
}
