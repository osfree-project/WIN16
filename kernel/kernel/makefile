#
# A Makefile for KRNL286.EXE
# (c) osFree project,
#



PROJ = kernel
TRGT = $(PROJ).exe
DESC = Windows/86 Kernel

srcfiles = $(p)ldboot$(e) $(p)hmemset$(e) $(p)kernel16$(e) $(p)global$(e) &
$(p)smallcode$(e) $(p)atom$(e) $(p)resource$(e) $(p)error$(e) &
$(p)profile$(e) $(p)strings$(e) $(p)rom$(e) $(p)file$(e) $(p)main$(e) $(p)local$(e) &
$(p)catch$(e) $(p)nls$(e) $(p)taskc$(e) $(p)enablec$(e) $(p)div$(e) &
$(p)mthread$(e) $(p)protect$(e) $(p)printf$(e) $(p)ginterf$(e) $(p)selector$(e) $(p)task$(e) &
$(p)winexec$(e) $(p)dosbox$(e) 
# $(p)profstrg$(e)
ADD_ASMOPT =  -D?32BIT=0 -D?DEBUG=1 -D?REAL=1 -I=$(MYDIR).. -I=$(MYDIR)..$(SEP)..$(SEP)..$(SEP)macrolib
#            -Zi -Zd -Fl DEBUG
ADD_COPT =  -zW -s -0 -I=$(%WATCOM)$(SEP)h$(SEP)win
# -Zc
# -zW option not work because not windows ne target. Need to switch to NE
JWASM = 1
DEBUG = all
DEST        = os2$(SEP)mdos$(SEP)winos2$(SEP)system

!include $(%ROOT)tools/mk/appsdos.mk

.asm: $(MYDIR);$(MYDIR)..
.c: $(MYDIR)..


