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
/**
 * @file system.c
 * @brief SYSTEM.DRV implementation for Windows 3.x
 * @author Wine project
 * @license GNU LGPL v2.1+
 * @date 2025-08-14
 * @source wine-8.9/dlls/system.drv/system.c
 * 
 * @note Added BIOS debug output for critical functions
 */

/* Data Types */
#define FAR         __far
#define NEAR        __near
typedef unsigned short WORD;
typedef unsigned long DWORD;
typedef int BOOL;
typedef void (interrupt far *FARPROC)();
typedef unsigned int HANDLE;
typedef void FAR        *LPVOID;
#define WINAPI      __far __pascal


/* Constants */
#define TRUE 1
#define FALSE 0
#define NB_SYS_TIMERS 8
#define SYS_TIMER_RATE 54925  /* 54.925 ms (18.2 Hz) */
#define BIOS_DEBUG 1          /* Enable BIOS debug output */
#ifndef NULL
    #define NULL        0
#endif

/* Timer Structure */
typedef struct {
    FARPROC callback;  /* Callback function */
    long rate;         /* Timer interval in microseconds */
    long ticks;        /* Countdown to next trigger */
} SYSTEM_TIMER;

/* Global Variables */
static DWORD clockTick;               /* System uptime in microseconds */
static SYSTEM_TIMER SYS_Timers[NB_SYS_TIMERS]; /* Active timers */
static int SYS_NbTimers;              /* Number of active timers */
static BOOL SYS_timers_disabled;      /* Timer processing flag */
static void interrupt (*PrevTimerIntHandler)(); /* Original INT 8 handler */

/* Interrupt Helpers */
extern  void    disable( void );
#pragma aux disable = "cli";

extern  void    enable( void );
#pragma aux enable = "sti";

extern  void (__interrupt far *getvect( int ax ))();
#pragma aux getvect = \
    "mov ah, 35h" \
    "int 21h" \
    __parm [__ax] __value [__es __bx]


extern  void setvect( int, void (__interrupt far *)());
#pragma aux setvect = \
    "push ds" \
    "mov ds, cx" \
    "mov ah, 25h" \
    "int 21h" \
    "pop ds" \
    __parm __caller [__ax] [__cx __dx]

/**
 * @brief BIOS Debug Output
 * @param str Null-terminated string to display
 * 
 * @note Uses INT 10h/0Eh to print to screen in real mode.
 *       Preserves all registers.
 */
static void debug_print(const char near * sstr) {
//label loop;
//label test;
    #if BIOS_DEBUG
    _asm {
        push ax
        push bx
        push si
        mov  ah, 0Eh     ; BIOS teletype function
        xor  bh, bh       ; Page 0
        mov  si, sstr      ; Load string pointer
        jmp  @test
@loop:
        lodsb             ; Load next char into AL
        int  10h          ; Print character
@test:
        cmp  byte ptr [si], 0
        jne  @loop
        pop  si
        pop  bx
        pop  ax
    }
    #endif
}

/**
 * @brief Timer Interrupt Handler (INT 8h)
 * 
 * Processes system timers and maintains time reference.
 * Chains to original BIOS timer handler.
 */
static void interrupt far SYSTEM_TimerTick(void) {
    int i;
    
    if (!SYS_timers_disabled) {
      clockTick=clockTick+SYS_TIMER_RATE/1000;
        
        for (i = 0; i < NB_SYS_TIMERS; i++) {
            if (!SYS_Timers[i].callback) continue;
            
            SYS_Timers[i].ticks -= SYS_TIMER_RATE;
            if (SYS_Timers[i].ticks <= 0) {
                SYS_Timers[i].ticks += SYS_Timers[i].rate;
                /* Debug: Show timer activation */
                #if BIOS_DEBUG
                _asm {
                    push ax
                    push bx
                    mov  ah, 0Eh
                    mov  al, '.'      ; Show timer tick
                    xor  bh, bh
                    int  10h
                    pop  bx
                    pop  ax
                }
                #endif
                SYS_Timers[i].callback();
            }
        }
    }
    
    /* Chain to original handler */
    (*PrevTimerIntHandler)();
}

/**
 * @brief Install custom timer handler
 */
static void SYSTEM_StartTicks(void) {
    disable();
    debug_print("SYS: Installing timer handler\r\n");
    PrevTimerIntHandler = getvect(8);
    setvect(8, SYSTEM_TimerTick);
    enable();
}


/**********************************************************************
 *           SYSTEM_StopTicks
 *
 * Stop the system tick timer.
 * @brief Restore original timer handler
 */
static void SYSTEM_StopTicks(void) {
    disable();
    debug_print("SYS: Restoring timer handler\r\n");
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
 *
 * @brief Create a system timer
 * @param rate Timer interval in milliseconds
 * @param proc Callback function
 * @return Timer ID (1-8) or 0 on failure
 * 
 * @note Minimum effective rate is 55ms due to hardware limitations
 */
WORD WINAPI CreateSystemTimer(WORD rate, FARPROC proc) {
    int i;
    
    if (!proc) return 0;
    
    for (i = 0; i < NB_SYS_TIMERS; i++) {
        if (SYS_Timers[i].callback) continue;
        
        SYS_Timers[i].rate = (unsigned int)rate * 1000;
        if (SYS_Timers[i].rate < SYS_TIMER_RATE) {
            SYS_Timers[i].rate = SYS_TIMER_RATE;
        }
        SYS_Timers[i].ticks = SYS_Timers[i].rate;
        SYS_Timers[i].callback = proc;
        
        if (SYS_NbTimers++ == 0) SYSTEM_StartTicks();
        
        /* Debug output */
        #if BIOS_DEBUG
        {
            char buf[40];
//            sprintf(buf, "SYS: Timer %d created @ %d ms\r\n", i+1, rate);
            debug_print("SYS: Timer created\r\n"/*buf*/);
        }
        #endif
        
        return i + 1;
    }
    debug_print("SYS: Timer creation failed!\r\n");
    return 0;
}


/***********************************************************************
 *           KillSystemTimer   (SYSTEM.3)
 *
 * Note: do not confuse this function with USER.182
 *
 * @brief Destroy a system timer
 * @param timer Timer ID to destroy
 * @return 1 on success, 0 on failure
 */
WORD WINAPI KillSystemTimer(WORD timer) {
    int idx;
    
    if (timer < 1 || timer > NB_SYS_TIMERS) return 0;
    idx = timer - 1;
    
    if (!SYS_Timers[idx].callback) return 0;
    
    SYS_Timers[idx].callback = NULL;
    
    if (--SYS_NbTimers == 0) SYSTEM_StopTicks();
    
    /* Debug output */
    #if BIOS_DEBUG
    {
        char buf[30];
        //sprintf(buf, "SYS: Timer %d killed\r\n", timer);
        debug_print("SYS: Timer killed\r\n"/*buf*/);
    }
    #endif
    
    return 1;
}

/**
/***********************************************************************
 * @brief Enable timer processing
 *           EnableSystemTimers   (SYSTEM.4)
 */
void WINAPI EnableSystemTimers(void) {
    debug_print("SYS: Timers enabled\r\n");
    SYS_timers_disabled = FALSE;
}

/**
 * @brief Disable timer processing
 *           DisableSystemTimers   (SYSTEM.5)
 */
void WINAPI DisableSystemTimers(void) {
    debug_print("SYS: Timers disabled\r\n");
    SYS_timers_disabled = TRUE;
}

/**
 * @brief Get system uptime in milliseconds
 *           GetSystemMSecCount (SYSTEM.6)
 */
DWORD WINAPI GetSystemMSecCount(void) {
    return clockTick;
}



/* FPU Functions (80287/80387) */
WORD WINAPI Get80x87SaveSize(void)
{
     return 94;

}

void WINAPI Save80x87State(char *buffer) {
    _asm {
        les bx, buffer
        fsave es:[bx]
    }
}

void WINAPI Restore80x87State(const char *buffer) {
    _asm {
        les bx, buffer
        frstor es:[bx]
    }
}

/**
 * @brief A20 line management (stub)
 * @note Not implemented for PC/XT systems
 */
void WINAPI A20_Proc(WORD action) {
    debug_print("SYS: A20_Proc called (stub)\r\n");
}

/***********************************************************************
 *           Driver Entry Point
 **********************************************************************/

/**
 * @brief Driver initialization
 * @param hinstDLL DLL instance handle
 * @param fdwReason Reason for call (process attach/detach)
 * @param lpvReserved Reserved
 * @return TRUE on success
 */
BOOL WINAPI LibMain(HANDLE hinstDLL, DWORD fdwReason, LPVOID lpvReserved) {
    int i;
_asm{
        mov  ah, 0Eh     ; BIOS teletype function
        xor  bh, bh       ; Page 0
	mov al, '#'
	int 10h
}    
    if (fdwReason == 1) {  /* DLL_PROCESS_ATTACH */
        debug_print("SYSTEM.DRV initializing\r\n");
        
        for (i = 0; i < NB_SYS_TIMERS; i++) {
            SYS_Timers[i].callback = 0;
            SYS_Timers[i].rate = 0;
            SYS_Timers[i].ticks = 0;
        }
        SYS_NbTimers = 0;
        clockTick = 0;
        SYS_timers_disabled = FALSE;
        
        debug_print("SYS: Ready\r\n");
    }
    return TRUE;
}
