#
# A Makefile for WinOS/2 PIF Editor
# (c) osFree project,
#

PROJ  = pifedit1
PROJ1 = pifedit
TRGT = $(PROJ1).exe
DESC = Windows Program Manager
srcfiles = $(p)qpifedit$(e) $(p)pif$(e)

# defines additional options for C compiler
ADD_COPT = -sg -DDEBUG=1
ADD_LINKOPT = LIB commdlg.lib

CLEAN_ADD = *.mbr
HEAPSIZE = 4k
STACKSIZE = 8k

!include $(%ROOT)tools/mk/appsw16.mk

TARGETS = $(PATH)$(PROJ1).exe

.ico: $(MYDIR)res

$(PATH)$(PROJ1).exe: $(PATH)$(PROJ).exe $(MYDIR)qpifedit.rc
 @$(SAY) RESCMP   $^. $(LOG)
 @wrc -q -bt=windows $]@ $[@ -fe=$@ -fo=$^@ -i=$(MYDIR) -i=$(%WATCOM)$(SEP)h$(SEP)win
