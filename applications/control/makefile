#
# A Makefile for WinOS/2 Program Manager
# (c) osFree project,
#

PROJ  = control1
PROJ1 = control
TRGT = $(PROJ1).exe
DESC = osFree Janus Control Panel
srcfiles = $(p)control$(e) 

# defines additional options for C compiler
ADD_COPT = -sg -0

EXPORTS = MainWndProc.1

!include $(%ROOT)tools/mk/appsw16.mk

TARGETS = $(PATH)$(PROJ1).exe


.ico: $(MYDIR)res

$(PATH)$(PROJ1).exe: $(PATH)$(PROJ).exe $(MYDIR)$(PROJ1).rc
 @$(SAY) RESCMP   $^. $(LOG)
 @$(RC) -q -30 -bt=windows $]@ $[@ -fe=$@ -fo=$^@ -i=$(MYDIR) -i=$(%WATCOM)$(SEP)h$(SEP)win
