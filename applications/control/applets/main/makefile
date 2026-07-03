#
# A Makefile for lzexpand.dll
# (c) osFree project,
#

PROJ     = main
TRGT     = $(PROJ).cpl
DESC     = Main CPL
srcfiles = $(p)main$(e) $(p)mouse$(e) $(p)keyboard$(e) $(p)time$(e) $(p)desktop$(e) $(p)inter$(e) $(p)currency$(e)
ADD_COPT = -ms -0 -sg -I=$(MYDIR)$(SEP)..$(SEP).. -I=$(MYDIR)$(SEP)..$(SEP)..$(SEP)..$(SEP)include
ADD_LINKOPT = lib nlsapi.lib

# linking dll file
DLL     = 1
EXPORTS = CPlApplet       .1, &
	MouseDlgProc.2, &
	DesktopDlgProc.3, &
	SystemTimerProc.4, &
	DateTimeDlgProc.5, &
	AnalogClockSubclassProc.6, &
	CalendarSubclassProc.7, &
	InternationalDlgProc.8, &
	TimeFmtDlgProc.9, &
	DateFmtDlgProc.10, &
	CurrencyFmtDlgProc.11

IMPORTS =   CreateSystemTimer SYSTEM.2, &
  KillSystemTimer SYSTEM.3

HEAPSIZE = 30k

DEST = os2$(SEP)mdos$(SEP)winos2$(SEP)system

!include $(%ROOT)tools/mk/appsw16.mk

TARGETS = $(PATH)$(PROJ).cpl

.ico: $(MYDIR)res

$(PATH)$(PROJ).cpl: $(PATH)$(PROJ).dll $(MYDIR)$(PROJ).rc
 @$(SAY) RESCMP   $^. $(LOG)
 @$(RC) -q -30 -bt=windows $]@ $[@ -fe=$@ -fo=$^@ -i=$(MYDIR) -i=$(%WATCOM)$(SEP)h$(SEP)win -i=$(MYDIR)$(SEP)..$(SEP)..$(SEP)..$(SEP)include
