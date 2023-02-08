#
# A Makefile for WinOS/2 Program Manager
# (c) osFree project,
#

PROJ  = notepad1
PROJ1 = notepad
TRGT = $(PROJ1).exe
DESC = Windows Notepad
srcfiles = $(p)main$(e) $(p)dialog$(e)

# defines additional options for C compiler
#ADD_COPT = -0 -sg -zw -bw -bg -d3 -db -hw -ml -od #-pl
ADD_COPT = -sg -DDEBUG=1
#DEBUG    = watcom
IMPORTS  = CHOOSEFONT     COMMDLG.15, &
           PRINTDLG       COMMDLG.20
#IMPORTS  = GETOPENFILENAME     COMMDLG.1, &
#           GETCURRENTDIRECTORY KERNEL.411
#           GETCURRENTDIRECTORY WINSMSG.20
#RESOURCE = $(PATH)rsrc.res
CLEAN_ADD = *.mbr

!include $(%ROOT)tools/mk/appsw16.mk

TARGETS = $(PATH)$(PROJ1).exe # subdirs

#$(PATH)$(PROJ1).res: $(PATH)rsrc.rc
# @$(SAY) WINRES   $^. $(LOG)
# @winres $^< -I $(%WATCOM)$(SEP)h$(SEP)win -o $^@

.ico: $(MYDIR)res

$(PATH)$(PROJ1).exe: $(PATH)$(PROJ).exe $(MYDIR)rsrc.rc
 @$(SAY) RESCMP   $^. $(LOG)
 @wrc -q -bt=windows $]@ $[@ -fe=$@ -fo=$^@ -i=$(MYDIR) -i=$(%WATCOM)$(SEP)h$(SEP)win
