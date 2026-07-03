# $Id: makefile,v 1.3 2004/08/21 14:49:22 prokushev Exp $
#
# Makefile for common shared libs

PROJ = nlsapi
TRGT = $(PROJ).lib
srcfiles = $(p)nlsapi$(e)
ADD_COPT = -I=$(MYDIR)..$(SEP)..$(SEP)include

!include $(%ROOT)tools/mk/libsw16.mk

$(TARGETS): $(OBJS)
 @$(MAKE) $(MAKEOPT) library=$(TARGETS) library

