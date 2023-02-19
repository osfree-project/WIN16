code	segment
	org	0h

tablesize	dw	tableend - tablestart

tablestart:

	include xlat915.inc

tableend:

; This is seems to be optional
	include	copyright.inc

code	ends

end
