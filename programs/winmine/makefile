#
# A Makefile for WinOS/2 Program Manager
# (c) osFree project,
#

PROJ  = winmine1
PROJ1 = winmine
TRGT = $(PROJ1).exe
DESC = Windows Mine Sweeper
srcfiles = $(p)winemine$(e) $(p)winemdlg$(e)

# defines additional options for C compiler
ADD_COPT = -sg -DDEBUG=1
EXPORTS =      MainProc, &
               CustomDlgProc, &
               CongratsDlgProc, &
               TimesDlgProc, &
               AboutDlgProc

HEAPSIZE = 8k
STACKSIZE = 24k

!include $(%ROOT)tools/mk/appsw16.mk

TARGETS = $(PATH)$(PROJ1).exe

.ico: $(MYDIR)res

$(PATH)$(PROJ1).exe: $(PATH)$(PROJ).exe $(MYDIR)winemine.rc
 @$(SAY) RESCMP   $^. $(LOG)
 @wrc -q -bt=windows $]@ $[@ -fe=$@ -fo=$^@ -i=$(MYDIR) -i=$(%WATCOM)$(SEP)h$(SEP)win
