#
# A Makefile for KERNEL.EXE
# (c) osFree project,
#



PROJ = kernel
TRGT = $(PROJ).exe
DESC = Windows Real Mode Kernel

srcfiles = $(p)ldboot$(e) $(p)hmemset$(e) $(p)profstrg$(e) $(p)kernel16$(e) $(p)global$(e) &
$(p)smallcode$(e) $(p)atom$(e) $(p)memset$(e) $(p)memcpy$(e) $(p)resource$(e) &
$(p)profile$(e) $(p)strings$(e) $(p)rom$(e) $(p)file$(e) $(p)main$(e) $(p)localc$(e) &
$(p)ginterf$(e) $(p)catch$(e) $(p)nls$(e) $(p)taskc$(e) &
$(p)mthread$(e) $(p)selectorc$(e) $(p)ld$(e) $(p)task$(e) $(p)enablec$(e) #$(p)enable$(e) 

ADD_ASMOPT = -Zi -Zd -Fl -D?32BIT=0 -D?DEBUG=1 -D?REAL=1 -I=$(MYDIR).. -I=$(MYDIR)..$(SEP)..$(SEP)..$(SEP)macrolib
ADD_COPT = -d3 -s -0 -I=$(%WATCOM)$(SEP)h$(SEP)win #-p
JWASM = 1
DEBUG = all
DEST        = os2$(SEP)mdos$(SEP)winos2$(SEP)system

!include $(%ROOT)/mk/appsdos.mk

.asm: $(MYDIR);$(MYDIR)..
.c: $(MYDIR)..

