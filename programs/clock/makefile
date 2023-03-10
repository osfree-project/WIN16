#
# A Makefile for WinOS/2 Program Manager
# (c) osFree project,
#

PROJ  = clock1
PROJ1 = clock
TRGT = $(PROJ1).exe
DESC = Windows clock
srcfiles = $(p)main$(e) $(p)winclock$(e)

# defines additional options for C compiler
ADD_COPT = -sg #-DDEBUG=1
IMPORTS  = CHOOSEFONT     COMMDLG.15

CLEAN_ADD = *.mbr

!include $(%ROOT)tools/mk/appsw16.mk

TARGETS = $(PATH)$(PROJ1).exe


.ico: $(MYDIR)res

$(PATH)$(PROJ1).exe: $(PATH)$(PROJ).exe $(MYDIR)clock.rc
 @$(SAY) RESCMP   $^. $(LOG)
 @wrc -q -bt=windows $]@ $[@ -fe=$@ -fo=$^@ -i=$(MYDIR) -i=$(%WATCOM)$(SEP)h$(SEP)win
