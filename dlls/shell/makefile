#
# A Makefile for shell.dll
# (c) osFree project,
# author, date
#

PROJ     = shell1
PROJ1    = shell
TRGT     = $(PROJ1).dll
DESC     = Windows Shell API Library
srcfiles = $(p)GlobalData$(e) $(p)About$(e) $(p)Environment$(e) $(p)AtomEx$(e) $(p)Strings$(e) $(p)Shell$(e) $(p)Registry$(e) $(p)DragDrop$(e) $(p)Exec$(e) $(p)Hook$(e) # $(p)Icon$(e)
# linking dll file
DLL     = 1
HEAPSIZE= 16k
EXPORTS = &
	RegOpenKey				.1, &
	RegCreateKey			.2, &
	RegCloseKey				.3, &
	RegDeleteKey			.4, &
	RegSetValue				.5, &
	RegQueryValue			.6, &
	RegEnumKey				.7, &
	WEP						.8 RESIDENT, &
	DragAcceptFiles			.9, &
	DragQueryFile			.11, &
	DragFinish				.12, &
	DragQueryPoint			.13, &
	ShellExecute			.20, &
	FindExecutable			.21, &
	ShellAbout				.22, &
	AboutDlgProc			.33, &
	DoEnvironmentSubst		.37, &
	FindEnvironmentString	.38, &
	DLLEntryPoint			.101, &
	RegisterShellHook		.102, &
	ShellHookProc			.103

#
#	ExtractIcon				.34, &
#	ExtractAssociatedIcon	.36, &
#	InternalExtractIcon		.39, &

# not implemented yet

#	EXTRACTICONEX.40, &
# here is 9x serie APIs
#	RESTARTDIALOG.157, &
#	DRIVETYPE.262, &
#	SHFORMATDRIVE .400, &
#	SHCHECKDRIVE .401
  

DEST = os2$(SEP)mdos$(SEP)winos2$(SEP)system

!include $(%ROOT)tools/mk/appsw16.mk

TARGETS = $(PATH)$(PROJ1).dll

.ico: $(MYDIR)res

$(PATH)$(PROJ1).dll: $(PATH)$(PROJ).dll $(MYDIR)$(PROJ1).rc
 @$(SAY) RESCMP   $^. $(LOG)
 @$(RC) -q -bt=windows $]@ $[@ -fe=$@ -fo=$^@ -i=$(MYDIR) -i=$(%WATCOM)$(SEP)h$(SEP)win
