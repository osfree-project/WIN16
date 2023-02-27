#
# A Makefile for toolhelp.dll
# (c) osFree project,
# author, date
#

PROJ     = toolhelp
TRGT     = $(PROJ).dll
DESC     = Windows Toolhelp API Library
srcfiles = $(p)toolhelp$(e)
# linking dll file
DLL     = 1
EXPORTS = &
	GlobalHandleToSel.50, &
	GlobalFirst.51, &
	GlobalNext.52, &
	GlobalInfo.53, &
	GlobalEntryHandle.54, &
	GlobalEntryModule.55, &
	LocalInfo.56, &
	LocalFirst.57, &
	LocalNext.58, &
	ModuleFirst.59, &
	ModuleNext.60, &
	ModuleFindName.61, &
	ModuleFindHandle.62, &
	TaskFirst.63, &
	TaskNext.64, &
	TaskFindHandle.65, &
	StackTraceFirst.66, &
	StackTraceCSIPFirst.67, &
	StackTraceNext.68, &
	ClassFirst.69, &
	ClassNext.70, &
	SystemHeapInfo.71, &
	MemManInfo.72, &
	NotifyRegister.73, &
	NotifyUnregister.74, &
	InterruptRegister.75, &
	InterruptUnRegister.76, &
	TerminateApp.77, &
	MemoryRead.78, &
	MemoryWrite.79, &
	TimerCount.80, &
	TASKSETCSIP.81, &
	TASKGETCSIP.82, &
	TASKSWITCH.83, &
	Local32Info.84, &
	Local32First.85, &
	Local32Next.86

# 83 is last for 3.10

DEST = os2$(SEP)mdos$(SEP)winos2$(SEP)system
#ADD_COPT=-pl

!include $(%ROOT)tools/mk/appsw16.mk
