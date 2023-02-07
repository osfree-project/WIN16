#
# A Makefile for WinOS/2 Program Manager
# (c) osFree project,
#

PROJ  = expand
TRGT = $(PROJ).exe
DESC = Windows clock
srcfiles = $(p)expand$(e)

# defines additional options for C compiler
#ADD_COPT = -0 -sg -zw -bw -bg -d3 -db -hw -ml -od #-pl
ADD_COPT = -sg -DDEBUG=1
#DEBUG    = watcom

#           GETCURRENTDIRECTORY WINSMSG.20
#RESOURCE = $(PATH)rsrc.res
CLEAN_ADD = *.mbr

!include $(%ROOT)tools/mk/appsw16.mk

TARGETS = $(PATH)$(PROJ).exe

