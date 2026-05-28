#
# A Makefile for WinOS/2 Program Manager
# (c) osFree project,
#

PROJ  = reversi1
PROJ1 = reversi
TRGT = $(PROJ1).exe
DESC = Windows reversi
srcfiles = $(p)othello$(e)

# defines additional options for C compiler
ADD_COPT = -sg -0
#ADD_LINKOPT = LIB commdlg.lib
EXPORTS =WndProc.1, dpProdInfo.2, dpEve.3

!include $(%ROOT)tools/mk/appsw16.mk

TARGETS = $(PATH)$(PROJ1).exe


.ico: $(MYDIR)res

$(PATH)$(PROJ1).exe: $(PATH)$(PROJ).exe $(MYDIR)$(PROJ1).rc
 @$(SAY) RESCMP   $^. $(LOG)
 @$(RC) -q -30 -bt=windows $]@ $[@ -fe=$@ -fo=$^@ -i=$(MYDIR) -i=$(%WATCOM)$(SEP)h$(SEP)win
