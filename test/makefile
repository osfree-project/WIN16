#
# A Makefile for WinOS/2 Program Manager
# (c) osFree project,
#

PROJ  = sysdir
TRGT      = $(PROJ).exe
srcfiles = $(p)sysdir$(e)

# defines additional options for C compiler
ADD_COPT = -DSTANDALONE
#DEBUG    = watcom
#IMPORTS  = CHOOSEFONT     COMMDLG.15

#           GETCURRENTDIRECTORY WINSMSG.20
#RESOURCE = $(PATH)rsrc.res
CLEAN_ADD = *.mbr
STACKSIZE = 34096
#HEAPSIZE = 50960

!include $(%ROOT)tools/mk/appsw16.mk

