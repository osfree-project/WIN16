#
# A Makefile for WinOS/2 Program Manager
# (c) osFree project,
#

PROJ  = notepad1
PROJ1 = notepad
TRGT = $(PROJ1).exe
DESC = Windows Notepad
srcfiles = $(p)main$(e) $(p)dialog$(e)

ADD_COPT = -sg -DDEBUG=1
IMPORTS  = GETOPENFILENAME     COMMDLG.1, &
           CHOOSEFONT     COMMDLG.15, &
           PRINTDLG       COMMDLG.20, &
           GETCURRENTDIRECTORY KERNEL.411

CLEAN_ADD = *.mbr
HEAPSIZE = 4k
STACKSIZE = 8k

!include $(%ROOT)tools/mk/appsw16.mk

TARGETS = $(PATH)$(PROJ1).exe # subdirs

#$(PATH)$(PROJ1).res: $(PATH)rsrc.rc
# @$(SAY) WINRES   $^. $(LOG)
# @winres $^< -I $(%WATCOM)$(SEP)h$(SEP)win -o $^@

.ico: $(MYDIR)res

$(PATH)$(PROJ1).exe: $(PATH)$(PROJ).exe $(MYDIR)rsrc.rc
 @$(SAY) RESCMP   $^. $(LOG)
 @wrc -q -bt=windows $]@ $[@ -fe=$@ -fo=$^@ -i=$(MYDIR) -i=$(%WATCOM)$(SEP)h$(SEP)win
