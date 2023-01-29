#
# A Makefile for shell.dll
# (c) osFree project,
# author, date
#

PROJ     = shell
TRGT     = $(PROJ).dll
DESC     = Windows Shell API Library
srcfiles = $(p)Registry$(e)
# linking dll file
DLL     = 1
EXPORTS = &
	RegOpenKey.1, &
	RegCreateKey.2, &
	RegCloseKey.3, &
	RegDeleteKey.4, &
	RegSetValue	.5, &
	RegQueryValue	.6, &
	RegEnumKey	.7, &
	DragAcceptFiles .9, &
	DragQueryFile   .11, &
	DragFinish      .12, &
	DragQueryPoint  .13, &
	ShellExecute	.20, &
	FindExecutable	.21, &
	ShellAbout	.22, &
	ExtractIcon	.34, &
	ExtractAssociatedIcon	.36, &
	DosEnvSubst	.37, &
	RegisterShellHook.102

# WEP.8 Need this?

OPTIONS = heapsize=32K libfile libentry.obj
DEST = os2$(SEP)mdos$(SEP)winos2$(SEP)system

!include $(%ROOT)tools/mk/appsw16.mk
