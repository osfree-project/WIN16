code	segment
	org	0h
start:
tablesize	dw	tableend - tablestart

tablestart:

	include xlat865.inc

tableend:

; This is seems to be optional
	include	copyright.inc

code	ends

end	start
