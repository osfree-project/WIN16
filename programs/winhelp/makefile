#
# A Makefile for WinOS/2 Help
# (c) osFree project,
#

PROJ  = winhelp1
PROJ1 = winhelp
TRGT = $(PROJ1).exe
DESC = Windows Help
srcfiles = &
	$(p)winhelp$(e) &
	$(p)hlpfile$(e) &
	$(p)macro$(e) &
	$(p)string$(e) &
	$(p)callback$(e) &
	$(PATH)macro_lex$(e)


# defines additional options for C compiler
#ADD_COPT = -0 -sg -zw -bw -bg -d3 -db -hw -ml -od #-pl
ADD_COPT = -sg -zc -dDEBUG #-dSTRICT
#DEBUG    = watcom
IMPORTS  = GETOPENFILENAME     COMMDLG.1, &
           PRINTDLG       COMMDLG.20
CLEAN_ADD = *.mbr
HEAPSIZE = 4k
STACKSIZE = 8k

!include $(%ROOT)tools/mk/appsw16.mk

TARGETS = $(PATH)$(PROJ1).exe # subdirs


.ico: $(MYDIR)res

$(PATH)macro_lex$(e): $(PATH)macro_lex.c

$(PATH)macro_lex.c: $(MYDIR)macro_lex.l

$(PATH)$(PROJ1).exe: $(PATH)$(PROJ).exe $(MYDIR)rsrc.rc
 @$(SAY) RESCMP   $^. $(LOG)
 @wrc -q -bt=windows $]@ $[@ -fe=$@ -fo=$^@ -i=$(MYDIR) -i=$(%WATCOM)$(SEP)h$(SEP)win
