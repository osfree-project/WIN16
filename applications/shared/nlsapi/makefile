# $Id: makefile,v 1.3 2004/08/21 14:49:22 prokushev Exp $
#
# Makefile for common shared libs

PROJ = nlsapi
TRGT = $(PROJ).lib
srcfiles = $(p)nlsapi_enum$(e) $(p)nlsapi_locale$(e) $(p)nlsapi_main$(e) $(p)nlsapi_utils$(e) $(p)nlsapi_num$(e) $(p)nlsapi_currency$(e)
ADD_COPT = -0 -I=$(MYDIR)..$(SEP)..$(SEP)include -DSTATIC
ADD_LINKOPT = lib setupinf.lib

!include $(%ROOT)tools/mk/libsw16.mk

$(TARGETS): $(OBJS)
 @$(MAKE) $(MAKEOPT) library=$(TARGETS) library

