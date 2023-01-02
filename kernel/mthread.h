/* MicroThread V2.4

  A minimal Turbo C multithreading library for DOS.
  Copyright 1993,1994,1995 J Ting. Please feel free to use
  this code in anyway that you see fit. Just don't hold me
  responsible for ANYTHING. The only guarantee that I will
  give is that the code will crash from time to time. You have
  been warned!

  Comments and suggestions to :
      Internet Email : j.ting@wlv.ac.uk

      Otherwise      : Jeffrey Ting
                       University of Wolverhampton
                       School of Computing & I. T.
                       Wulfruna Street
                       Wolverhampton WV1 1SB
                       U.K.
*/

#ifndef MTHREAD_H
#define MTHREAD_H
#define MTHREAD 24

#ifndef __STDIO_H
#include <stdio.h>
#endif

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

extern void setcbrk( int ax );
#pragma aux setcbrk = \
        "mov ah, 0x33" \
        "int 21h"         \
    __parm __caller     [__ax] \

/* pointer type to a thread function */
typedef int (far *PThreadFunc) (void far *pArg);

/* The semaphore type. */
typedef unsigned int Semaphore;

/* MicroThread termination codes.
   Returned by MTStartMultiThreading() */
enum MTReturnCode { NORMAL, DEADLOCKED, CTRLBREAK,
                    NOT_INITIALISED,TOO_DEEP_CRITICAL_NEST };

/* These are the functions you can call */

/* This initialised the MicroThread system,
   but doesn't start multithreading */
void MTInitialise(void);

/* Adds a new thread to the READY queue */
int  MTAddNewThread(PThreadFunc pThreadFunc,unsigned priority,
                    unsigned sizeArg, void *pArg);

/* This is just a convenience to avoid casting */
#define MTAddThread(f,p) MTAddNewThread((PThreadFunc)f,p,0,NULL)
#define MTAddArgThread(f,p,s,a) MTAddNewThread((PThreadFunc)f,p,s,a)

/* Starts multi-threading */
enum MTReturnCode MTStartMultiThreading(void);

/* Kill a thread */
void MTKillThread(unsigned threadID);

/* Kills the current thread */
void MTEndThread(void);

/* Kills all known threads dead*/
void MTEndMultiThreading(void);

/* Creates a semaphore dor application use. Returns 0 on error */
Semaphore MTCreateSema(void);

/* Destroys a semaphore.
   Destroying a semaphore still in use is an error!
   returns 1 on success and 0 on failure. */
unsigned MTDestroySema(Semaphore semaphore);

/* waits for a semaphore */
void MTWait(Semaphore Semaphore);

/* waits for a semaphore */
int MTTestAndSet(Semaphore Semaphore);

/* signals a semaphore, wakes up a thread waiting for
   the semaphore */
void MTSignal(Semaphore Semaphore);

/* enters critical section.
   Go to single-threading mode temporarily for calling function
   Zero return value indicates failure - too many nested
   critical sections - > MAX_CRITICAL_NESTING  */
unsigned MTEnterCritical(void);

/* leaves critcal section.
   Reverts back to mutli-threading. */
void MTLeaveCritical(void);

/* Elective yield */
void MTYield(void);

/* Enables/disables preemption.
   Must not be called whilst mutli-threading */
void MTSetPreemptive(int bPreempt);

/* Return the semaphoire used to serialise access to
   Turbo C's runtime libraries */
Semaphore MTGetCRTLibSema(void);

/* returns the number of clock ticks since MTStartMultiThreading.
   One second is approx. 18.2 clock ticks */
unsigned long MTGetClockTick(void);

/* Put the current thread to sleep for a length of time */
void MTSleep(unsigned long ticks);

unsigned MTGetMailboxHandle(char *mbxName);
int MTSendMsg(unsigned mailboxHnd, unsigned msgSize, void *pMsg);
int MTReceiveMsg(unsigned mailboxHnd, unsigned bufferSize, void *pBuffer);
unsigned MTCreateMailbox(char *name);
unsigned MTDestroyMailbox(unsigned mailboxHnd);


/* These are C runtime library replacements.
   They are basically semaphored-wrappers around
   the C runtime library functions */
int MTfprintf (FILE *stream, const char *format,...);
int MTprintf (const char *format,...);
int MTfputs(const char *s, FILE *stream);
int MTfputc(int c, FILE *stream);
FILE *MTfopen(const char *filename, const char *mode);
int MTfclose(FILE *stream);
int MTfeof(FILE *stream);
int MTsprintf (char *buffer, const char *format,...);
char *MTfgets(char *s, int n, FILE *stream);
int MTfgetc(FILE *stream);
int MTxyputc(int x, int y, int c);
int MTxyputs(int x, int y, const char *s);
int MTxyprintf (int x, int y, const char *format, ...);
int MTkbhit(void);
int MTgetch(void);
char * MTgets(char *buf);
void MTclrscr(void);
int MTrandom(int num);
void *MTmalloc(size_t size);
void MTfree(void *block);
char *MTstrcpy(char *source, char *dest);
size_t MTfread(void *ptr, size_t size, size_t n, FILE *stream);
size_t MTfwrite(const void *ptr, size_t size, size_t n, FILE*stream);

#ifndef MTCRTLIB_C
#define MTputchar(c) MTfputc(c,stdout)
#define MTputc(c)    MTfputc(c,stdout)
#define MTputs(s)    MTfputs(s,stdout)
#endif

#endif /* MTHREAD_H */

