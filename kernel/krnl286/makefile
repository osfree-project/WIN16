#
# A Makefile for KRNL286.EXE
# (c) osFree project,
#



PROJ = krnl286
TRGT = $(PROJ).exe
DESC = Windows/286 Kernel

srcfiles = $(p)ldboot$(e) $(p)hmemset$(e) $(p)profstrg$(e) $(p)kernel16$(e) $(p)global$(e) &
$(p)smallcode$(e) $(p)atom$(e) $(p)resource$(e) $(p)error$(e) &
$(p)profile$(e) $(p)strings$(e) $(p)rom$(e) $(p)file$(e) $(p)main$(e) $(p)local$(e) &
$(p)catch$(e) $(p)nls$(e) $(p)taskc$(e) $(p)enablec$(e) $(p)div$(e) &
$(p)mthread$(e) $(p)protect$(e) $(p)printf$(e) $(p)2ginterf$(e) $(p)selector$(e) $(p)task$(e) &
$(p)winexec$(e)

ADD_ASMOPT =  -D?32BIT=0 -D?DEBUG=1 -D?REAL=0 -I=$(MYDIR).. -I=$(MYDIR)..$(SEP)..$(SEP)..$(SEP)macrolib
#            -Zi -Zd -Fl DEBUG
ADD_COPT =  -s -Zc -2 -I=$(%WATCOM)$(SEP)h$(SEP)win
#          -d3 ^^^ DEBUG
JWASM = 1
DEBUG = all
DEST        = os2$(SEP)mdos$(SEP)winos2$(SEP)system

!include $(%ROOT)tools/mk/appsdos.mk

.asm: $(MYDIR);$(MYDIR)..
.c: $(MYDIR)..


