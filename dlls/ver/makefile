#
# A Makefile for shell.dll
# (c) osFree project,
# author, date
#

PROJ     = ver
TRGT     = $(PROJ).dll
DESC     = Windows ver API Library
srcfiles = $(p)version$(e) $(p)movefile$(e)
# linking dll file
DLL     = 1
EXPORTS = &
	GetFileResourceSize.2, &
	GetFileResource.3, &
	GetFileVersionInfoSize.6, &
	GetFileVersionInfo.7, &
	VerFindFile.8, &
	VerInstallFile.9, &
	VerLanguageName.10, &
	VerQueryValue.11


# not implemented yet
#	DragAcceptFiles .9, &
#	DragQueryFile   .11, &
#	DragFinish      .12, &
#	DragQueryPoint  .13, &
#	ABOUTDLGPROC.33, &
#	ExtractAssociatedIcon	.36, &
#	INTERNALEXTRACTICON.39, &
#	EXTRACTICONEX.40, &
#	RegisterShellHook.102, &
#	SHELLHOOKPROC.103, &
#	RESTARTDIALOG.157, &
#	DRIVETYPE.262, &
#	SHFORMATDRIVE .400, &
#	SHCHECKDRIVE .401
  
# WEP.8 Need this?

OPTIONS = heapsize=32K lib lzexpand.lib libfile libentry.obj
DEST = os2$(SEP)mdos$(SEP)winos2$(SEP)system
#ADD_COPT=-pl

!include $(%ROOT)tools/mk/appsw16.mk
