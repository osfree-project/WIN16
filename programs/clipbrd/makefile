#
# A Makefile for WinOS/2 Clipboard Viewer
# (c) osFree project,
#

PROJ  = clipbrd1
PROJ1 = clipbrd
TRGT = $(PROJ1).exe
DESC = Windows Clipboard Viewer
srcfiles = $(p)clipbrd$(e) $(p)cliputils$(e) $(p)winutils$(e) $(p)fileutils$(e) $(p)scrollutils$(e)

# defines additional options for C compiler
ADD_COPT = -sg 
ADD_LINKOPT = LIB commdlg.lib

CLEAN_ADD = *.mbr

!include $(%ROOT)tools/mk/appsw16.mk

TARGETS = $(PATH)$(PROJ1).exe


.ico: $(MYDIR)res

$(PATH)$(PROJ1).exe: $(PATH)$(PROJ).exe $(MYDIR)clipbrd.rc
 @$(SAY) RESCMP   $^. $(LOG)
 @wrc -q -bt=windows $]@ $[@ -fe=$@ -fo=$^@ -i=$(MYDIR) -i=$(%WATCOM)$(SEP)h$(SEP)win
