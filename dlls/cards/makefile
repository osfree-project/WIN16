#
# A Makefile for shell.dll
# (c) osFree project,
# author, date
#

PROJ     = cards
PROJ1    = cards1
TRGT     = $(PROJ).dll
DESC     = Windows Cards API Library
srcfiles = $(p)cards$(e)
ADD_COPT = -dCARDSTYLE_DEFAULT

# linking dll file
DLL     = 1
EXPORTS = &
	WEP.1, &
	cdtAnimate.2, &
	cdtDraw.3, &
	cdtDrawExt.4, &
	cdtInit.5, &
	cdtTerm.6

DEST = os2$(SEP)mdos$(SEP)winos2

!include $(%ROOT)tools/mk/appsw16.mk

TARGETS = $(PATH)$(PROJ1).dll

.ico: $(MYDIR)res

$(PATH)$(PROJ1).dll: $(PATH)$(PROJ).dll $(MYDIR)cards.rc
 @$(SAY) RESCMP   $^. $(LOG)
 @wrc -dCARDSTYLE_DEFAULT -q -bt=windows $]@ $[@ -fe=$@ -fo=$^@ -i=$(MYDIR) -i=$(%WATCOM)$(SEP)h$(SEP)win
