#
# A Makefile for KERNEL.EXE
# (c) osFree project,
#



PROJ = kernel
TRGT = $(PROJ).exe
DESC = Windows Real Mode Kernel
srcfiles = $(p)ldboot$(e) $(p)hmemset$(e) $(p)lstrcat$(e) $(p)lstrcpy$(e) $(p)lstrlen$(e) $(p)profstrg$(e) $(p)kernel16$(e) &
$(p)smallcode$(e) $(p)atom$(e) $(p)lstrcpyn$(e) $(p)memset$(e) $(p)toupper$(e) $(p)strnicmp$(e) $(p)memcpy$(e) &
$(p)profile$(e) $(p)ansi$(e) $(p)tolower$(e) $(p)lstrcmp$(e) $(p)rom$(e) $(p)file$(e) $(p)main$(e) &
$(p)strlen$(e) $(p)atoi$(e) $(p)ginterf$(e) $(p)local$(e) $(p)catch$(e) $(p)nls$(e) $(p)selector$(e) &
$(p)mthread$(e) $(p)istask$(e) $(p)GetExeVersion$(e)  $(p)longptradd$(e)

ADD_ASMOPT = -Zi -Zd -Fl -D?32BIT=0 -D?DEBUG=1 -D?REAL=1 -I=$(MYDIR).. -I=$(MYDIR)..$(SEP)..$(SEP)..$(SEP)macrolib
ADD_COPT = -d3 -s -0 -I=$(%WATCOM)$(SEP)h$(SEP)win
JWASM = 1
DEBUG = all
DEST        = os2$(SEP)mdos$(SEP)winos2$(SEP)system
DEBUG = all

!include $(%ROOT)/mk/appsdos.mk

.asm: $(MYDIR);$(MYDIR)..
.c: $(MYDIR)..

