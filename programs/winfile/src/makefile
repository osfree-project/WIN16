#
# A Makefile for WinOS/2 File Manager
# (c) osFree project,
#

PROJ  = winfile1
PROJ1 = winfile
TRGT = $(PROJ1).exe
DESC = Windows File Manager
#       $(p)dbg$(e) &
#       $(p)lfn$(e) &
#       $(p)lfnmisc$(e) &
srcfiles = &
       $(p)winfile$(e) &
       $(p)numfmt$(e) &
       $(p)res$(e) &
       $(p)suggest$(e) &
       $(p)tbar$(e) &
       $(p)treectl$(e) &
       $(p)wfassoc$(e) &
       $(p)wfchgnot$(e) &
       $(p)wfcomman$(e) &
       $(p)wfcopy$(e) &
       $(p)wfdir$(e) &
       $(p)wfdirrd$(e) &
       $(p)wfdirsrc$(e) &
       $(p)wfdlgs$(e) &
       $(p)wfdlgs2$(e) &
       $(p)wfdlgs3$(e) &
       $(p)wfdos$(e) &
       $(p)wfdrives$(e) &
       $(p)wfext$(e) &
       $(p)wffile$(e) &
       $(p)wfinfo$(e) &
       $(p)wfinit$(e) &
       $(p)wfmem$(e) &
       $(p)wfprint$(e) &
       $(p)wfsearch$(e) &
       $(p)wftree$(e) &
       $(p)wfutil$(e) &
       $(p)wnetcaps$(e)


# defines additional options for C compiler
#ADD_COPT = -0 -sg -zw -bw -bg -d3 -db -hw -ml -od #-pl
ADD_COPT = -sg -dDEBUG=1 -uUNICODE  
#DEBUG    = watcom
IMPORTS  = GETOPENFILENAME     COMMDLG.1, &
           GETCURRENTDIRECTORY KERNEL.411
#           GETCURRENTDIRECTORY WINSMSG.20
#RESOURCE = $(PATH)rsrc.res
CLEAN_ADD = *.mbr

!include $(%ROOT)tools/mk/appsw16.mk

TARGETS = $(PATH)$(PROJ1).exe # subdirs

#$(PATH)$(PROJ1).res: $(PATH)rsrc.rc
# @$(SAY) WINRES   $^. $(LOG)
# @winres $^< -I $(%WATCOM)$(SEP)h$(SEP)win -o $^@

.ico: $(MYDIR)res

$(PATH)$(PROJ1).exe: $(PATH)$(PROJ).exe $(MYDIR)res.rc
 @$(SAY) RESCMP   $^. $(LOG)
 @wrc -q -bt=windows $]@ $[@ -fe=$@ -fo=$^@ -i=$(MYDIR) -i=$(%WATCOM)$(SEP)h$(SEP)win
