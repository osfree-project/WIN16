code	segment
	org	0h

tablesize	dw	tableend - tablestart

public tablestart
tablestart:

	include xlat437.inc

tableend:

; This is seems to be optional
	include	copyright.inc

code	ends

end