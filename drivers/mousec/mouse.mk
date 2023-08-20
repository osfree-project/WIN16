# $Id: makefile,v 1.1 2004/08/16 06:27:05 prokushev Exp $

PORT_NAME = win16$(SEP)mouse
PORT_TYPE = git
PORT_URL  = https://git.javispedro.com/cgit/vbmouse.git/
PORT_REV  = 
#PORT_PATCHES  = attrib.diff

!include $(%ROOT)tools/mk/port.mk
