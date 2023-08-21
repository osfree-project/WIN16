#
# A Makefile for WinOS/2 Program Manager
# (c) osFree project,
#

PROJ  = file
TRGT      = $(PROJ).exe
srcfiles = $(p)$(PROJ)$(e)

# defines additional options for C compiler
ADD_COPT = -DSTANDALONE -D_X86_ -zW -e25 -zq -od -d2 -w4 -wcd=126 -wcd=102 -wcd=112 -oaxt -bg -i..
#DEBUG    = watcom
#IMPORTS  = CHOOSEFONT     COMMDLG.15

#           GETCURRENTDIRECTORY WINSMSG.20
#RESOURCE = $(PATH)rsrc.res
CLEAN_ADD = *.mbr
STACKSIZE = 2048
HEAPSIZE = 2048

!include $(%ROOT)tools/mk/appsw16.mk

