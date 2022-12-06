;
; Process Data Base
;

PDB	STRUCT
PDB_INT20		DW	?	;            /* 00 int 20h instruction */
PDB_NEXTPARAGRAPH	DW	?	;    /* 02 Segment of next paragraph */
PDB_RESERVED1		DB	?	;
PDB_DISPATCHER		DB	5 DUP (?);    /* 05 Long call to DOS */
PDB_SAVEDINT22		DD	?	;       /* 0a Saved int 22h handler */
PDB_SAVEDINT23		DD	?	;       /* 0e Saved int 23h handler */
PDB_SAVEDINT24		DD	?	;       /* 12 Saved int 24h handler */
PDB_PARENTPSP		DW	?	;        /* 16 Selector of parent PSP */
PDB_FILEHANDLES		DB	20 DUP (?);  /* 18 Open file handles */
PDB_ENVIRONMENT		DW	?	;      /* 2c Selector of environment */
PDB_SAVESTACK		DD	?	;        /* 2e SS:SP on last int21 call */
PDB_NBFILES		DW	?	;          /* 32 Number of file handles */
PDB_FILEHANDLESPTR	DD	?	;   /* 34 Pointer to file handle table */
PDB_HFILEHANDLES	DW	?	;     /* 38 Handle to fileHandlesPtr */
PDB_RESERVED3		DW	17 DUP (?);
PDB_FCB1		DB	16 DUP (?);         /* 5c First FCB */
PDB_FCB2		DB	20 DUP (?);         /* 6c Second FCB */
PDB_CMDLINE		DB	128 DUP (?);     /* 80 Command-line (first byte is len)*/
PDB_PADDING		DB	16 DUP (?);      /* Some apps access beyond the end of the cmd line */
PDB	ENDS
