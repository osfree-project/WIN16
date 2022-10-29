/* MicroThread V2.5

  A minimal Borland/Turbo C multithreading library for DOS program.
  Written and placed into the public domain by J Ting. Please read
  the files README.TXT and MTHREAD.TXT for more information.

  Comments and suggestions to :
      Internet Email : j.ting@wlv.ac.uk

      Otherwise      : Jeffrey Ting
                       University of Wolverhampton
                       School of Computing & I. T.
                       Wulfruna Street
                       Wolverhampton WV1 1SB
                       U.K.
*/

/* MTHREAD.C - Main file for the kernel functions. This is all you
   need if you are not using any of Turbo C's runtime libraries. If you
   do, then you will also need MTCRTLIB.C
*/


#define MTHREAD_C 25
#include "stdio.h"
#include "stdarg.h"
#include "dos.h"
#include "conio.h"
#include "stdlib.h"
#include "mem.h"
#include "mthread.h"

#include <win16.h>

void ctrlbrk(int (*fptr)(void))
{
	setvect(0x23, (void (__interrupt __far *))fptr);
}


/* Clock intrreupt vector */
#define TIMER_INT_VECT 8

/* DOS INT21 interrupt vector. Not used in this version */
#define DOS21_INT_VECT 0x21

/* Maximum nesting for critical sections */
#define MAX_CRITICAL_NESTING 64000L

/* Change these to suit your application */
#define MAX_THREADS  40
#define MAX_SEMAPHORES  20
#define MAX_MAILBOXES  40
#define MAILBOX_NAME_LEN 32
#define THREAD_STACK_SIZE 8192

/* Macro wrappers to make it easier to use the general EnqueueThread,
   DequeueThread and ExtractThread functions */
#define EnqueueReady(ID) \
  EnqueueThread(ID,&ReadyQHeads[threads[ID].priority], \
                &ReadyQTails[threads[ID].priority])

#define DequeueReady(priority) \
  DequeueThread(&ReadyQHeads[priority],&ReadyQTails[priority])

#define ExtractReady(ID) \
  ExtractThread(ID,&ReadyQHeads[threads[ID].priority], \
                &ReadyQTails[threads[ID].priority])


#define EnqueueSema(threadID, semaID)\
  EnqueueThread(curThreadID, \
                &(semaphores[semaID].QHead), \
                &(semaphores[semaID].QTail))

#define DequeueSema(semaID) \
  DequeueThread(&(semaphores[semaID].QHead),&(semaphores[semaID].QTail))

#define ExtractSema(ID) \
  ExtractThread(ID,&(semaphores[threads[ID].semaphore].QHead), \
                &(semaphores[threads[ID].semaphore].QTail))


/* The various states of the threads */
enum ThreadStatus {INVALID, CURRENT, READY, WAITING, SLEEPING, TERMINATED};

/* Structure to hold the CPU registers on the runtime stack */
typedef struct {
  unsigned bp;
  unsigned di;
  unsigned si;
  unsigned ds;
  unsigned es;
  unsigned dx;
  unsigned cx;
  unsigned bx;
  unsigned ax;
  unsigned ip;
  unsigned cs;
  unsigned flags;
} RegsOnStack;


/* Thread execution context */
typedef struct {
  unsigned sp;  /* stack pointer */
  unsigned ss;  /* stack segment */
  unsigned priority;  /* priority: 1,2,3,4 or 5 */
  enum ThreadStatus status;
  unsigned semaphore; /* blocking semaphore */
  void *pOwnStack;
  PThreadFunc pFunc; /* pointer to the thread function */
  void *pArg;   /* pointer argument passed to the thread function */
  void *pMsg;
  unsigned msgSize;
  unsigned long wakeUpTime;  /* time to wake the sleeping thread up */
  unsigned prevID; /* ID of previous thread in the linked-list queue */
  unsigned nextID; /* ID of next thread in the linked-list queue */
} ThreadContext;


/* The Thread List - implemented as an array of thread execution contexts
   so that the thread IDs correspond to the array indices */
ThreadContext threads[MAX_THREADS];

/* Semaphores */
typedef struct{
  unsigned value; /* this is incremented on each wait */
  unsigned inUse; /* Nonzero if in used, non-zero otherwise*/
  unsigned QHead; /* head of queue of threads waiting on this semaphore*/
  unsigned QTail; /* tail of above queue */
}SemaphoreInfo;

/* Array of available semaphores, index is ID */
SemaphoreInfo semaphores[MAX_SEMAPHORES];


/* Mailboxes */
typedef struct{
  char name[MAILBOX_NAME_LEN];
  unsigned numReceivers; /* num of receivers is waiting */
  unsigned numSenders; /*num of senders waiting */
  unsigned inUse; /* Nonzero if in used, non-zero otherwise*/
  unsigned QHead; /* head of queue of threads waiting on this mbx*/
  unsigned QTail; /* tail of above queue */
} MailBoxInfo;

/* Array of available mailboxes */
MailBoxInfo mailboxes[MAX_MAILBOXES];

/* Forward prototypes */
static void TheScheduler(void);
static void interrupt StopMultiThreading(void);
static void interrupt DispatchCurThread(void);
static void interrupt GotoScheduler(void);
void interrupt (*PrevTimerIntHandler)(void);
static void WakeUpThreads(unsigned semaphore);
static void TidyUp(void);
static int  IsDeadLocked(void);
static void interrupt RealStart(void);
void EnqueueThread(unsigned threadID, unsigned *pQHead, unsigned *pQTail);
unsigned DequeueThread(unsigned *pQHead, unsigned *pQTail);
void WakeUpSleepingThreads(void);


/* Static varaiables for MicroThread management */

/* Various stack segment and pointer storage variables */
static unsigned originalSS, originalSP,schedulerSS, schedulerSP, tmpSS, tmpSP;

/* The current thread ID */
static unsigned curThreadID = 0;

/* lock count for critical sections */
static unsigned iCriticalSection=0;

/* flag to indicate whether currently multithreading */
static char bMultiThreading = 0;

/* is MicroThread initialised ? */
static char bMTInitialised=0;

/* Ctrl-Break flag */
static char bCtrlBreak=0;

/* The READY queues - one for each priority*/
static unsigned ReadyQHeads[6], ReadyQTails[6];

static unsigned sleepQHead, sleepQTail;

/* MicroThread can be preemptive or non-preemptive,
   This flag control the behaviour.
   Use MTSetPreemptive() to change it */
static char bMTPreemptive=1;

/* Mthread return code */
static enum MTReturnCode returnCode=NORMAL;

/* The semaphore used to seralise access top C runtime library functions */
Semaphore semaCRunTimeLib=0;

/* pointer to InDOS flag for checking to see if
   we are in a dos service rountine somewhere */
static char far * pInDosFlag;

/* clock ticks since MThread started */
static unsigned long clockTick=0;

/*static unsigned long nextWakeUpTick=0;*/

/* Scheduling policy. Change ChooseNextThread() to change scheduling
  policy */
static unsigned scheduleMap[15]={5,4,3,5,4,2,5,3,4,5,1,4,5,3,2};
static unsigned priorityIdx=15;

unsigned ChooseNextThread(void)
{
  unsigned nextID;
  unsigned loopCounter=0;
  while(1){
    priorityIdx++;
    if (priorityIdx >=15){
      priorityIdx=0;
      loopCounter++;
    }
    nextID = DequeueReady(scheduleMap[priorityIdx]);
    if (nextID==0){
      if(loopCounter>2) /* Gone round twice? */
        return 0; /* No READY threads! Possibly deadlocked. */
    }
    else
      return nextID;
  }
}

/* The scheduler itself */
static void TheScheduler(void)
{
  unsigned nextThreadID;

  while(1){
    /* Check that the current thrad is a valid one, if so
       join it to the back of the READY queue */
    if(curThreadID > 0 && threads[curThreadID].status==CURRENT){
      threads[curThreadID].status = READY;
      EnqueueReady(curThreadID);
    }
    /* check that the user hasn't hit Ctrl-Break */
    if(bCtrlBreak){
      bMultiThreading = 0;
      returnCode = CTRLBREAK;
    }
    /* if any threads are sleeping, check and
       wake up the apropriate threads */
    if(sleepQHead){
      WakeUpSleepingThreads();
    }
    /* Call ChooseNextThread to select a thread for dispatching */
    if(bMultiThreading) {
      nextThreadID = ChooseNextThread();
      curThreadID = nextThreadID;
      /* Check for deadlock if no threads are sleeping */
      if (sleepQHead==0){
        if(nextThreadID==0){ /* Deadlocked. End multi-threading */
          bMultiThreading = 0;
          returnCode = DEADLOCKED;
        }
        else {
          curThreadID = nextThreadID;
        }
      }
    }
    /* Check to see if we are ending multi-threading */
    if(!bMultiThreading) {
      StopMultiThreading();
    }
    /* If a thread is found, switch to it */
    if (nextThreadID) DispatchCurThread();
  }
}


/* The dispatcher, switches from the scheduler to the current thread */
static void interrupt DispatchCurThread(void)
{
  //putchar('T');
  disable();
  __asm {mov schedulerSS, ss}
//  schedulerSS = _SS;
  __asm {mov schedulerSP, sp}
//  schedulerSP = _SP;
  tmpSS=threads[curThreadID].ss;
  __asm {mov ss, tmpSS}
//  _SS = threads[curThreadID].ss;
  tmpSP=threads[curThreadID].sp;
  __asm {mov sp, tmpSP}
//  _SP = threads[curThreadID].sp;
  threads[curThreadID].status = CURRENT;
  enable();
}


/* go from thread to scheduler */
static void interrupt GotoScheduler(void)
{
  disable();
  __asm{mov tmpSS, ss}
  threads[curThreadID].ss=tmpSS;
  __asm{mov tmpSP, sp}
  threads[curThreadID].sp=tmpSP;
  //threads[curThreadID].ss = _SS;
//  threads[curThreadID].sp = _SP;
  __asm{mov ss, schedulerSS}
  __asm{mov sp, schedulerSP}
//  _SS = schedulerSS;
//  _SP = schedulerSP;
  enable();
}


/* Our own Control-Break handler - it sets a flag
   so that we know when to stop multi-threading */
static int CtrlBreak(void)
{
   if (bMultiThreading) {
     bCtrlBreak=1;
     return 1;
   }
   else
     return 0;
}


/* The replacement timer interrupt handler.
   Switches from the current thread to the scheduler */
static void interrupt OurTimerIntHandler(void)
{
  /* Saves the current thread's stack seg and pointer.
     Set the stack segment and pointer to point to the scheduler's
     and the scheduler will resume running after the timer interrupt */
    clockTick++;
    __asm {mov tmpSS, ss}
    __asm {mov tmpSP, sp}

    if (*pInDosFlag==0){ /* Are we running a DOS routine somewhere? */
      if (iCriticalSection==0){ /* Are we in a critical section? */
        if (tmpSS != schedulerSS ){ /* Are we already in our scheduler? */
          if(tmpSS==threads[curThreadID].ss){/* Are we in a valid thread and*/
            threads[curThreadID].sp = tmpSP; /* not a strange TSR somewhere?*/
              __asm{mov ss, schedulerSS}
//            _SS = schedulerSS;    /* None of the above! So we can */
              __asm{mov sp, schedulerSP}
//            _SP = schedulerSP;    /* switch to the scheduler */
          }
        }
      }
    }
  (*PrevTimerIntHandler)(); /* Call the call the original interrupt handler */
}


/* Stick a thread to the back of a queue */
void EnqueueThread(unsigned threadID, unsigned *pQHead, unsigned *pQTail)
{
  if (*pQHead ==0){ /* nothing in the queue */
    *pQHead = threadID;
    *pQTail = threadID;
    threads[threadID].prevID=0;
  }
  else{
    threads[*pQTail].nextID = threadID;
    threads[threadID].prevID = *pQTail;
    *pQTail = threadID;
  }
  threads[threadID].nextID=0;
}


/* take the thread at the head of the queue off  and return its ID */
unsigned DequeueThread(unsigned *pQHead, unsigned *pQTail)
{
  unsigned threadID=0;

  if (*pQHead ==0){ /* nothing in the queue */
    return 0;       /* error! */
  }
  else if (*pQHead == *pQTail){ /* one item in the queue */
    threadID = *pQHead;
    *pQHead = *pQTail = 0;
  }
  else {
    threadID = *pQHead;
    *pQHead = threads[*pQHead].nextID;
    threads[*pQHead].prevID=0;
  }
  return threadID;
}


/* this function assumes that threadID is the queue.
   If not, we are in trouble! */
void ExtractThread(unsigned threadID, unsigned *pQHead, unsigned *pQTail)
{
  unsigned prevID=0, nextID=0;

  prevID = threads[threadID].prevID;
  nextID = threads[threadID].nextID;
  if (prevID==0){
    *pQHead=nextID;
  }
  else {
    threads[prevID].nextID = nextID;
  }

  if (nextID==0){
    *pQTail=prevID;
  }
  else {
    threads[nextID].prevID = prevID;
  }
}



/* Big red button. Puts the original stack back,
   and takes out out interrupt handler */
static void interrupt StopMultiThreading(void)
{
  disable();
  __asm { mov ss, originalSS}
//  _SS = originalSS;
  __asm { mov sp, originalSP}
//  _SP = originalSP;
  if (bMTPreemptive){
    setvect(TIMER_INT_VECT, PrevTimerIntHandler);
  }
  enable();
}


/* Tidy up - return MicroThread to the uninitialised state.
   A bit of an overkill, but better be safe than sorry! */
static void TidyUp(void)
{
  int i;

  /* free up all the thread stacks */
  for(i=0; i<MAX_THREADS; i++) {
    if(threads[i].pOwnStack != NULL) {
//      free(threads[i].pOwnStack);
      GlobalDosFree(threads[i].pOwnStack);
      threads[i].pOwnStack = NULL;
    }
    if(threads[i].pArg != NULL) {
//      free(threads[i].pArg);
      GlobalDosFree(threads[i].pArg);
      threads[i].pArg = NULL;
    }
    threads[i].status = INVALID;
    threads[i].semaphore = 0;
    threads[i].ss=0;
    threads[i].nextID=0;
  }
  for(i=0; i<MAX_SEMAPHORES; i++) {
    semaphores[i].value=0;
    semaphores[i].inUse=0;
    semaphores[i].QHead=0;
    semaphores[i].QTail=0;
  }
  for(i=1; i<=5; i++){
    ReadyQHeads[i]=0;
    ReadyQTails[i]=0;
  }
  curThreadID = 0;
  iCriticalSection=0;
  bMultiThreading = 0;
  bMTInitialised=0;
  bCtrlBreak=0;
  semaCRunTimeLib=0;
}


/* Obvious, but not currently used in this version */
static int IsDeadLocked(void)
{
  int i;
  /* If there are no threads READY to run, we are DeadLocked */
  for(i=1; i<MAX_THREADS; i++)
    if (threads[i].status==READY) return 0;
  return 1;
}

/* A shell for the thread function. This is to allow us to pass arguments
   to the thread function, and to call MTEndThread() when the thread
   function returns */
void ThreadShell(void)
{
  (*(threads[curThreadID].pFunc))(threads[curThreadID].pArg);
  MTEndThread();
}

static void CopyMem(char *pDest, char *pSource, unsigned nBytes)
{

  unsigned i;
  for(i=0; i<nBytes; i++){
    *pDest++ = *pSource++;
  }
}

/* Adds a new thread. Can be used from a running thread.
   MicroThread has to be already initialised */
int MTAddNewThread(PThreadFunc pThreadFunc, unsigned priority,
                   unsigned sizeArg, void *pArg)
{
  RegsOnStack *pRegs;
  unsigned threadID, i;
  void *pNewStack, *pArgMem;

  if (bMTInitialised){
    MTWait(semaCRunTimeLib);
    MTEnterCritical();
    threadID = MAX_THREADS;
    /* look for a useable slot in the thread list */
    for(i=0; i<MAX_THREADS; i++){
      if (threads[i].status == INVALID
        ||threads[i].status == TERMINATED){
        threadID = i;
        break;
      }
    }
    if (threadID == MAX_THREADS){  /* Failed */
      MTLeaveCritical();
      MTSignal(semaCRunTimeLib);
      return 0;
    }
    /* allocate a new stack for the thread.*/
//    pNewStack = malloc(THREAD_STACK_SIZE + sizeof(RegsOnStack));
    pNewStack = GlobalDosAlloc(THREAD_STACK_SIZE + sizeof(RegsOnStack));
    if (pNewStack==NULL){
      MTLeaveCritical();
      MTSignal(semaCRunTimeLib);
      return 0;  /* failed */
    }
    if (sizeArg && pArg) {
//      pArgMem = malloc(sizeArg);
      pArgMem = GlobalDosAlloc(sizeArg);
      if (pArgMem==NULL){
//        free(pNewStack);
        GlobalDosFree(pNewStack);
        MTLeaveCritical();
        MTSignal(semaCRunTimeLib);
        return 0;  /* failed */
      }
      CopyMem(pArgMem, pArg, sizeArg);
    }
    else {
      pArgMem=NULL;
    }
    threads[threadID].pOwnStack = pNewStack;
    pRegs = (RegsOnStack *) threads[threadID].pOwnStack +
            THREAD_STACK_SIZE - sizeof(RegsOnStack);
    threads[threadID].sp = FP_OFF((RegsOnStack far *) pRegs);
    threads[threadID].ss = FP_SEG((RegsOnStack far *) pRegs);
    threads[threadID].pFunc = pThreadFunc;
    threads[threadID].pArg = pArgMem;
    pRegs->cs = FP_SEG(ThreadShell);
    pRegs->ip = FP_OFF(ThreadShell);

    __asm {mov tmpSS, ds}
    pRegs->ds=tmpSS;
//    pRegs->ds = _DS;
    __asm {mov tmpSS, es}
    pRegs->es=tmpSS;
//    pRegs->es = _ES;
    pRegs->flags = 0x200;
    threads[threadID].priority = priority;
    threads[threadID].status = READY;
    EnqueueReady(threadID);
    MTLeaveCritical();
    MTSignal(semaCRunTimeLib);
  }
  return threadID;
}


/* Kicks off the system by setting up the stack pointer to point
   to the scheduler, and patching in the timer interrupt vector
   if it is in preemptive mode */
static void interrupt RealStart(void)
{

  __asm{mov originalSS, ss}
//  originalSS = _SS;
  __asm{mov originalSP, sp}
//  originalSP = _SP;
  __asm{mov ss, schedulerSS}
//  _SS = schedulerSS;
  __asm{mov sp, schedulerSP}
//  _SP = schedulerSP;
  if (bMTPreemptive){
    disable();
    PrevTimerIntHandler = getvect(TIMER_INT_VECT);
    setvect(TIMER_INT_VECT, OurTimerIntHandler);
    enable();
  }
  bMultiThreading=1;
  returnCode = NORMAL;
}


/* A shell for RealStart, and takes care of tidying up
   when multi-threading stops */
enum MTReturnCode MTStartMultiThreading(void)
{
  union REGS regs;
  struct SREGS sregs;
  unsigned tmpes;
  unsigned tmpbx;


  __asm {
    mov ah, 34h
    int 21h
    mov tmpes, es
    mov tmpbx, bx
  }
  pInDosFlag=MK_FP(tmpes, tmpbx);
//  regs.h.ah=0x34;
//  int86x(0x21,&regs,&regs,&sregs);
//  pInDosFlag=MK_FP(sregs.es, regs.x.bx);
  setcbrk(1);
  ctrlbrk(CtrlBreak);
  if (bMTInitialised && !bMultiThreading){
    RealStart();
    TidyUp();
    return returnCode;
  }
  else
    return NOT_INITIALISED;
}



/* Stops a running thread and frees its stack */
void MTKillThread(unsigned threadID)
{
  MTWait(semaCRunTimeLib);
  MTEnterCritical();
  if(bMultiThreading && threadID <MAX_THREADS && threadID > 0){
    if(threadID != curThreadID){
      if(threads[threadID].status==READY){
        ExtractReady(threadID);
      }
      else if(threads[threadID].status==WAITING){
        ExtractSema(threadID);
      }
    }
    threads[threadID].status = TERMINATED;
    if(threads[threadID].pOwnStack)
//      free(threads[threadID].pOwnStack);
      GlobalDosFree(threads[threadID].pOwnStack);
    threads[threadID].pOwnStack = NULL;
    threads[threadID].ss=0;
    if(threads[threadID].pArg)
//      free(threads[threadID].pArg);
      GlobalDosFree(threads[threadID].pArg);
    threads[threadID].pArg = NULL;
    MTLeaveCritical();
    MTSignal(semaCRunTimeLib);
    GotoScheduler();
  }
  MTLeaveCritical();
  MTSignal(semaCRunTimeLib);
}


/* Called by a thread to kill itself */
void MTEndThread(void)
{
  MTKillThread(curThreadID);
}


/* Initialises the MicroThread system.
   Needs to be called before anything else
   or after MTEndMultiThreading()
   for subsequent runs */
void MTInitialise(void)
{
  int i;

  if (!bMTInitialised){
    curThreadID = 0;
    iCriticalSection=0;
    bMultiThreading = 0;
    bCtrlBreak=0;
    bMTPreemptive=1;
    returnCode=NORMAL;
    for(i=0; i<MAX_THREADS; i++) {
      threads[i].status = INVALID;
      threads[i].semaphore = 0;
      threads[i].pOwnStack = NULL;
      threads[i].ss=0;
      threads[i].pFunc=NULL;
      threads[i].pArg=NULL;
      threads[i].pMsg=NULL;
      threads[i].msgSize=0;
      threads[i].nextID=0;
      threads[i].prevID=0;
    }
    for(i=0; i<MAX_SEMAPHORES; i++) {
      semaphores[i].value=0;
      semaphores[i].inUse=0;
      semaphores[i].QHead=0;
      semaphores[i].QTail=0;
    }
    for(i=0; i<MAX_MAILBOXES; i++) {
      mailboxes[i].name[0]='\0';
      mailboxes[i].numSenders=0; /* this is incremented on each wait */
      mailboxes[i].numReceivers=0; /* this is incremented on each wait */
      mailboxes[i].inUse=0; /* Nonzero if in used, non-zero otherwise*/
      mailboxes[i].QHead=0; /* head of queue of threads waiting to send to this mbx*/
      mailboxes[i].QTail=0; /* tail of above queue */
    }
    for(i=1; i<=5; i++){
      ReadyQHeads[i]=0;
      ReadyQTails[i]=0;
    }
    sleepQHead=0;
    sleepQTail=0;
    bMTInitialised = 1;
    if (MTAddNewThread((PThreadFunc)TheScheduler,0,0,NULL) != 0){
      bMTInitialised = 0;
      return;
    }
    schedulerSS = threads[0].ss;
    schedulerSP = threads[0].sp;
    semaCRunTimeLib=MTCreateSema();
    clockTick=0L;
  }
//  directvideo=1;
}



/* Stops multi-threading - to be called by a running thread */
void MTEndMultiThreading(void)
{
  if (bMultiThreading){
    bMultiThreading = 0;
    GotoScheduler();
  }
}



/* allocates a unique semaphore
   id of 0 is used as an error condition */
Semaphore MTCreateSema(void)
{
  unsigned i;
  MTEnterCritical();
  for (i=1; i<MAX_SEMAPHORES; i++){
    if(semaphores[i].inUse==0){
      semaphores[i].inUse=1;
      MTLeaveCritical();
      return i;
    }
  }
  MTLeaveCritical();
  return 0;
}


/* Frees a semaphore.
   Freeing a semaphore still in use is an error! */
unsigned MTDestroySema(Semaphore semaID)
{
  MTEnterCritical();
  if(semaphores[semaID].value==0){ /* is it still in use? */
    semaphores[semaID].inUse=0;
    MTLeaveCritical();
    return 1; /* No */
  }
  MTLeaveCritical();
  return 0; /* Yes */
}



/* A non-waiting version of MTWait. sets the semaphore only
   if it is not set, otherwise it does nothing.
   Returns 1 on success and 0 on failure.*/
int MTTestAndSet(Semaphore semaID)
{
  int retval=0;
  MTEnterCritical();
  if (bMultiThreading && semaphores[semaID].inUse){
      threads[curThreadID].semaphore = semaID;
      if(semaphores[semaID].value==0){
        semaphores[semaID].value++;
        retval=1;
      }
  }
  MTLeaveCritical();
  return retval;
}


/* A thread must NOT wait for the same semaphore recursively */
void MTWait(Semaphore semaID)
{
  MTEnterCritical();
  if (bMultiThreading && semaphores[semaID].inUse){
      threads[curThreadID].semaphore = semaID;
      if(semaphores[semaID].value==0){
        semaphores[semaID].value++;
      }
      else{
        threads[curThreadID].status = WAITING;
        EnqueueSema(curThreadID,semaID);
        MTLeaveCritical();
        GotoScheduler();
      }
  }
  MTLeaveCritical();
}


/* Releases and signals that a semaphore is free */
void MTSignal(Semaphore semaID)
{
  MTEnterCritical();
  if (bMultiThreading && semaphores[semaID].inUse){
      if(semaphores[semaID].value>0){
        semaphores[semaID].value--;
        WakeUpThreads(semaID);
      }
      MTLeaveCritical();
      GotoScheduler();
  }
  MTLeaveCritical();
}

/* Resume a thread waiting for a semaphore.
   No real policy just FIFO */
static void WakeUpThreads(unsigned semaID)
{
  unsigned threadID;

  if(semaphores[semaID].QHead){
    threadID=DequeueSema(semaID);
    threads[threadID].status=READY;
    threads[threadID].semaphore=0;
    EnqueueReady(threadID);
  }
}

/* Enter a critical section - freezes all other threads */
unsigned MTEnterCritical(void)
{
  unsigned retval = 0;

  if (bMultiThreading){
    iCriticalSection++;
    retval = iCriticalSection;
    if (iCriticalSection== MAX_CRITICAL_NESTING){
      returnCode=TOO_DEEP_CRITICAL_NEST;
      StopMultiThreading();
    }
  }
  return retval;
}


/* Leaves critical section - resumes multithreading */
void MTLeaveCritical(void)
{
  if (bMultiThreading && iCriticalSection > 0)
    iCriticalSection--;
}


/* Elective yield for non-preemptive mode */
void MTYield(void)
{
  if (bMultiThreading)
    GotoScheduler();
}


/* Enables/disables preemption,
   but not when running! */
void MTSetPreemptive(int bPreempt)
{
  if ( !bMultiThreading)
    bMTPreemptive = bPreempt ? 1 : 0;
}


/* Return the semaphore used to serialise access to
   Turbo C's runtime libraries */
Semaphore MTGetCRTLibSema(void)
{
  return semaCRunTimeLib;
}

/* Returns the number of clock ticks since MTStartMultiThreading.
   One second is approx. 18.2 clock ticks */
unsigned long MTGetClockTick(void)
{
  return clockTick;
}

#if 0
void printsleepq(void)
{
  unsigned tid=sleepQHead;
  printf("\nHead(%u)=>",sleepQHead);
  while(tid){
    printf("(%u,%u,%u),",threads[tid].prevID,tid,threads[tid].nextID);
    if(sleepQTail==tid)
      printf("<=Tail(%u)",sleepQTail);
    tid=threads[tid].nextID;
  }
}
#endif

/* Puts the current thread to sleep for a length of time */
void MTSleep(unsigned long ticks)
{
   unsigned threadIdx, nextID;
   unsigned long wakeUpTime;
   MTEnterCritical();
   wakeUpTime =clockTick + ticks;
   threads[curThreadID].wakeUpTime=wakeUpTime;
   threads[curThreadID].status=SLEEPING;
   if (sleepQHead==0){
     sleepQHead=sleepQTail=curThreadID;
     threads[curThreadID].prevID=0;
     threads[curThreadID].nextID=0;
   }
   else {
     threadIdx=sleepQTail;
     while(threadIdx!=sleepQHead
       && threads[threadIdx].wakeUpTime > wakeUpTime){
       threadIdx=threads[threadIdx].prevID;
     }
     if(threadIdx==sleepQHead
       && threads[sleepQHead].wakeUpTime > wakeUpTime){
       sleepQHead=curThreadID;
       threads[curThreadID].prevID=0;
       threads[curThreadID].nextID=threadIdx;
       threads[threadIdx].prevID=curThreadID;
     }
     else{
       if(threadIdx==sleepQTail){
         sleepQTail=curThreadID;
       }
       threads[curThreadID].prevID=threadIdx;
       threads[curThreadID].nextID=nextID=threads[threadIdx].nextID;
       threads[threadIdx].nextID=curThreadID;
       if(sleepQTail != curThreadID)
         threads[nextID].prevID=curThreadID;
     }
   }
   MTLeaveCritical();
   GotoScheduler();
}


/* Checks and wakes up any sleeping thread that needs waking up */
void WakeUpSleepingThreads(void)
{
  unsigned threadID;
  while(sleepQHead && threads[sleepQHead].wakeUpTime <= clockTick){
    threadID = DequeueThread(&sleepQHead, &sleepQTail);
    threads[threadID].status=READY;
    EnqueueReady(threadID);
  }
}


/* allocates a mailbox given a name
   id of 0 is used as an error condition */
unsigned MTCreateMailbox(char *name)
{
  unsigned i,j;
  MTEnterCritical();
  for (i=1; i<MAX_MAILBOXES; i++){
    if(mailboxes[i].inUse==0){
      mailboxes[i].inUse=1;
      for(j=0; j<MAILBOX_NAME_LEN; j++){
        mailboxes[i].name[j]=name[j];
        if (name[j]=='\0') break;
      }
      mailboxes[i].name[MAILBOX_NAME_LEN-1]='\0';
      MTLeaveCritical();
      return i;
    }
  }
  MTLeaveCritical();
  return 0;
}

/* helper function */
int StrMatch(char *str1, char *str2)
{
  int retval=1;
  while (*str1 && *str2){
    if(*str1!=*str2){
      retval=0;
      break;
    }
    str1++;
    str2++;
  }
  return retval;
}

unsigned MTGetMailboxHandle(char *mbxName)
{
  unsigned i;
  for (i=1; i<MAX_MAILBOXES; i++){
    if(mailboxes[i].inUse){
      if(StrMatch(mbxName,mailboxes[i].name)){
        return i;
      }
    }
  }
  return 0;
}

int MTSendMsg(unsigned mailboxHnd, unsigned msgSize, void *pMsg)
{
    unsigned receiverId, copyMsgSize;
    int retval=0;

    MTEnterCritical();
    if(mailboxes[mailboxHnd].inUse){
      /* Are there any receivers waiting on this mailbox/ */
      if(mailboxes[mailboxHnd].numReceivers > 0){
        /* if so, remove it from the queue */
        receiverId=DequeueThread(&(mailboxes[mailboxHnd].QHead),
                                 &(mailboxes[mailboxHnd].QTail));
        /* check for size of receivers msg buffer being smaller */
        copyMsgSize=msgSize;
        if(threads[receiverId].msgSize < msgSize)
          copyMsgSize=threads[receiverId].msgSize;
        /* transfer the msg */
        CopyMem(threads[receiverId].pMsg, pMsg, copyMsgSize);
        threads[receiverId].msgSize=copyMsgSize;
        /* put the receiver into the ready queue */
        threads[receiverId].status = READY;
        EnqueueReady(receiverId);
        mailboxes[mailboxHnd].numReceivers--;
        retval=copyMsgSize;
      }
      /* no receivers, wait in the queue */
      else {
        threads[curThreadID].pMsg = pMsg;
        threads[curThreadID].msgSize = msgSize;
        threads[curThreadID].status = WAITING;
        EnqueueThread(curThreadID,&(mailboxes[mailboxHnd].QHead),
                                  &(mailboxes[mailboxHnd].QTail));
        mailboxes[mailboxHnd].numSenders++;
        MTLeaveCritical();
        GotoScheduler();
        retval = threads[curThreadID].msgSize;
      }
    }
    MTLeaveCritical();
    return retval;
}


int MTReceiveMsg(unsigned mailboxHnd, unsigned bufferSize, void *pBuffer)
{
    unsigned senderId, copyMsgSize;
    int retval=0;

    MTEnterCritical();
    if(mailboxes[mailboxHnd].inUse){
      /* Are there any senders waiting on this mailbox/ */
      if(mailboxes[mailboxHnd].numSenders > 0){
        /* if so, remove it from the queue */
        senderId=DequeueThread(&(mailboxes[mailboxHnd].QHead),
                               &(mailboxes[mailboxHnd].QTail));
        /* check for size of receivers msg buffer being smaller */
        copyMsgSize=bufferSize;
        if(threads[senderId].msgSize < bufferSize)
          copyMsgSize=threads[senderId].msgSize;
        /* transfer the msg */
        CopyMem(pBuffer, threads[senderId].pMsg, copyMsgSize);
        retval = threads[senderId].msgSize=copyMsgSize;
        /* put the receiver into the ready queue */
        threads[senderId].status = READY;
        EnqueueReady(senderId);
        mailboxes[mailboxHnd].numSenders--;
      }
      /* no senders, wait in the queue */
      else {
        threads[curThreadID].pMsg = pBuffer;
        threads[curThreadID].msgSize = bufferSize;
        threads[curThreadID].status = WAITING;
        EnqueueThread(curThreadID,&(mailboxes[mailboxHnd].QHead),
                                  &(mailboxes[mailboxHnd].QTail));
        mailboxes[mailboxHnd].numReceivers++;
        MTLeaveCritical();
        GotoScheduler();
        retval = threads[curThreadID].msgSize;
      }
    }
    MTLeaveCritical();
    return retval;
}


unsigned MTDestroyMailbox(unsigned mailboxHnd)
{
  MTEnterCritical();
  if(mailboxes[mailboxHnd].numReceivers
    || mailboxes[mailboxHnd].numSenders){ /* is it still in use? */
    mailboxes[mailboxHnd].inUse=0;
    MTLeaveCritical();
    return 1; /* No */
  }
  MTLeaveCritical();
  return 0; /* Yes */
}




