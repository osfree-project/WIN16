;
; Task Data Base
;

TDB	STRUCT
TDB_HNEXT		DW	?	;              /* 00 Selector of next TDB */
TDB_SP			DW	?	;              /* 02 Stack pointer of task */
TDB_SS			DW	?	;
TDB_NEVENTS		DW	?	;           /* 06 Events for this task */
TDB_PRIORITY		DW	?	;           /* 08 Task priority, -32..15 */
TDB_UNUSED1		DW	?	;            /* 0a */
TDB_HSELF		DW	?	;              /* 0c Selector of this TDB */
TDB_HPREVINSTANCE	DW	?	;      /* 0e Previous instance of module */
TDB_UNUSED2		DD	?	;            /* 10 */
TDB_CTRLWORD8087	DW	?	;       /* 14 80x87 control word */
TDB_FLAGS		DW	?	;              /* 16 Task flags */
TDB_ERROR_MODE		DW	?	;         /* 18 Error mode (see SetErrorMode)*/
TDB_VERSION		DW	?	;            /* 1a Expected Windows version */
TDB_HINSTANCE		DW	?	;          /* 1c Instance handle for task */
TDB_HMODULE		DW	?	;            /* 1e Module handle */
TDB_HQUEUE		DW	?	;             /* 20 Selector of task queue */
TDB_HPARENT		DW	?	;            /* 22 Selector of TDB of parent */
TDB_SIGNAL_FLAGS	DW	?	;       /* 24 Flags for signal handler */
TDB_SIGHANDLER		DD	?	;         /* 26 Signal handler */
TDB_USERHANDLER		DD	?	;        /* 2a USER signal handler */
TDB_DISCARDHANDLER	DD	?	;     /* 2e Handler for GlobalNotify() */
TDB_INT0		DD	?	;               /* 32 int 0 (divide by 0) handler */
TDB_INT2		DD	?	;               /* 36 int 2 (NMI) handler */
TDB_INT4		DD	?	;               /* 3a int 4 (INTO) handler */
TDB_INT6		DD	?	;               /* 3e int 6 (invalid opc) handler */
TDB_INT7		DD	?	;               /* 42 int 7 (coprocessor) handler */
TDB_INT3E		DD	?	;              /* 46 int 3e (80x87 emu) handler */
TDB_INT75		DD	?	;              /* 4a int 75 (80x87 error) handler */
TDB_COMPAT_FLAGS	DD	?	;       /* 4e Compatibility flags */
TDB_UNUSED4		DB	2 DUP (?);         /* 52 */
TDB_UNK			DD	?	;//struct _TEB *teb;             /* 54 Pointer to thread database */
TDB_UNUSED5		DB	8 DUP (?);         /* 58 */
TDB_HPDB		DW	?	;               /* 60 Selector of PDB (i.e. PSP) */
TDB_DTA			DD	?	;                /* 62 Current DTA */
TDB_CURDRIVE		DB	?	;           /* 66 Current drive */
TDB_CURDIR		DB	65 DUP (?);         /* 67 Current directory */
TDB_NCMDSHOW		DW	?	;           /* a8 cmdShow parameter to WinMain */
TDB_HYIELDTO		DW	?	;           /* aa Next task to schedule */
TDB_DLLS_TO_INIT	DD	?	;       /* ac Ptr to DLLs to initialize */
TDB_HCSALIAS		DW	?	;           /* b0 Code segment for this TDB */
TDB_THUNKS		DW	8*4 DUP (?);        /* b2 Make proc instance thunks */
TDB_MODULE_NAME		DB	8 DUP (?);     /* f2 Module name for task */
TDB_MAGIC		DW	?	;              /* fa TDB signature */
;    HANDLE    hEvent;             /* fc scheduler event handle */
;    PDB       pdb;                /* 100 PDB for this task */
TDB	ENDS
