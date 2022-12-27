#
# A Makefile for KRNL286.EXE
# (c) osFree project,
#



PROJ = krnl286
TRGT = $(PROJ).exe
DESC = Windows/286 Kernel

srcfiles = $(p)ldboot$(e) $(p)hmemset$(e) $(p)profstrg$(e) $(p)kernel16$(e) $(p)global$(e) &
$(p)smallcode$(e) $(p)atom$(e) $(p)memset$(e) $(p)memcpy$(e) $(p)resource$(e) &
$(p)profile$(e) $(p)strings$(e) $(p)rom$(e) $(p)file$(e) $(p)main$(e) $(p)localc$(e) &
$(p)catch$(e) $(p)nls$(e) $(p)taskc$(e) $(p)enablec$(e) &
$(p)mthread$(e) $(p)2protect$(e) $(p)2ginterf$(e) $(p)selectorc$(e) $(p)task$(e)  $(p)ld$(e)

ADD_ASMOPT = -Zi -Zd -Fl -D?32BIT=0 -D?DEBUG=1 -D?REAL=0 -I=$(MYDIR).. -I=$(MYDIR)..$(SEP)..$(SEP)..$(SEP)macrolib
#            ^^^^^^^^^^^ DEBUG
ADD_COPT = -d3 -s -2 -I=$(%WATCOM)$(SEP)h$(SEP)win
#          ^^^ DEBUG
JWASM = 1
DEBUG = all
DEST        = os2$(SEP)mdos$(SEP)winos2$(SEP)system

!include $(%ROOT)/mk/appsdos.mk

.asm: $(MYDIR);$(MYDIR)..
.c: $(MYDIR)..


