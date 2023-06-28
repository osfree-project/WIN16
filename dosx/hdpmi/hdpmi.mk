# $Id: makefile,v 1.1 2004/08/16 06:27:05 prokushev Exp $

PORT_NAME = dos$(SEP)hdpmi
PORT_TYPE = git
PORT_URL  = https://github.com/Baron-von-Riedesel/HX
PORT_REV  = v2.20
#PORT_PATCHES  = attrib.diff

!include $(%ROOT)tools/mk/port.mk
