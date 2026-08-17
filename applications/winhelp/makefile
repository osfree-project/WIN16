#
# A Makefile for osFree Janus Help
# (c) osFree project
#

DESCRIPTION = osFree Janus Help
SOURCES = winhelp hlpfile macro string callback macro_lex


# defines additional options for C compiler
#ADD_COPT = -0 -sg -zw -bw -bg -d3 -db -hw -ml -od #-pl
ADD_COPT = -zc -dDEBUG #-dSTRICT
IMPORTS  = GETOPENFILENAME     COMMDLG.1, &
           PRINTDLG       COMMDLG.20

!include $(%ROOT)tools/mk/build.mk

$(PATH)macro_lex$(e): $(PATH)macro_lex.c

$(PATH)macro_lex.c: $(MYDIR)macro_lex.l
