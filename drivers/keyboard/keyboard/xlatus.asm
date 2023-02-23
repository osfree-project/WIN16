_TEXT	segment word public 'CODE'
	org	0h

tablesize	dw	tableend - tablestart

public tablestart
tablestart:

	include xlat437.inc

tableend:

; This is seems to be optional
	include	copyright.inc

_TEXT	ends

end
