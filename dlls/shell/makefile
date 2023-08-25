#
# A Makefile for shell.dll
# (c) osFree project,
# author, date
#

PROJ     = shell
TRGT     = $(PROJ).dll
DESC     = Windows Shell API Library
srcfiles = $(p)atomex$(e) $(p)strings$(e) $(p)Shell$(e) $(p)About$(e) $(p)Registry$(e)  $(p)DragDrop$(e)
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
	ABOUTDLGPROC.33=ShellAboutHandler, &
	ExtractIcon	.34, &
	DOENVIRONMENTSUBST.37, &
	FINDENVIRONMENTSTRING.38, &
	DLLENTRYPOINT.101

# not implemented yet

#	ExtractAssociatedIcon	.36, &
#	INTERNALEXTRACTICON.39, &
#	EXTRACTICONEX.40, &
#	RegisterShellHook.102, &
#	SHELLHOOKPROC.103, &
# here is 9x serie APIs
#	RESTARTDIALOG.157, &
#	DRIVETYPE.262, &
#	SHFORMATDRIVE .400, &
#	SHCHECKDRIVE .401
  
# WEP.8 Need this?

DEST = os2$(SEP)mdos$(SEP)winos2$(SEP)system
#ADD_COPT=-pl

!include $(%ROOT)tools/mk/appsw16.mk
