code	segment
	org	0h

tablesize	dw	tableend - tablestart

tablestart:

	include xlat866.inc

tableend:

	include	copyright.inc

code	ends

end
