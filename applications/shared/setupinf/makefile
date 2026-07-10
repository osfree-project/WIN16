# $Id: makefile,v 1.3 2004/08/21 14:49:22 prokushev Exp $
#
# Makefile for common shared libs

PROJ = setupinf
TRGT = $(PROJ).lib
srcfiles = $(p)setupinf$(e) $(p)string_utils$(e)
ADD_COPT = -I=$(MYDIR)..$(SEP)..$(SEP)include #-ml


!include $(%ROOT)tools/mk/libsw16.mk

$(TARGETS): $(OBJS)
 @$(MAKE) $(MAKEOPT) library=$(TARGETS) library

