#include <win16.h>

#define HQUEUE HANDLE

/* Process database (i.e. a normal DOS PSP) */
typedef struct
{
    WORD      int20;            /* 00 int 20h instruction */
    WORD      nextParagraph;    /* 02 Segment of next paragraph */
    BYTE      reserved1;
    BYTE      dispatcher[5];    /* 05 Long call to DOS */
    FARPROC	savedint22;       /* 0a Saved int 22h handler */
    FARPROC savedint23;       /* 0e Saved int 23h handler */
    FARPROC savedint24;       /* 12 Saved int 24h handler */
    WORD      parentPSP;        /* 16 Selector of parent PSP */
    BYTE      fileHandles[20];  /* 18 Open file handles */
    HANDLE  environment;      /* 2c Selector of environment */
    DWORD     saveStack;        /* 2e SS:SP on last int21 call */
    WORD      nbFiles;          /* 32 Number of file handles */
    DWORD    fileHandlesPtr;   /* 34 Pointer to file handle table */
    HANDLE	hFileHandles;     /* 38 Handle to fileHandlesPtr */
    WORD      reserved3[17];
    BYTE      fcb1[16];         /* 5c First FCB */
    BYTE      fcb2[20];         /* 6c Second FCB */
    BYTE      cmdLine[128];     /* 80 Command-line (first byte is len)*/
    BYTE      padding[16];      /* Some apps access beyond the end of the cmd line */
} PDB;

/* Task database. See 'Windows Internals' p. 226.
 * Note that 16-bit OLE 2 libs like to read it directly
 * so we have to keep entry offsets as they are.
 */
typedef struct _TDB
{
    HTASK     hNext;              /* 00 Selector of next TDB */
    DWORD     ss_sp;              /* 02 Stack pointer of task */
    WORD      nEvents;            /* 06 Events for this task */
    int       priority;           /* 08 Task priority, -32..15 */
    WORD      unused1;            /* 0a */
    HTASK     hSelf;              /* 0c Selector of this TDB */
    HANDLE    hPrevInstance;      /* 0e Previous instance of module */
    DWORD     unused2;            /* 10 */
    WORD      ctrlword8087;       /* 14 80x87 control word */
    WORD      flags;              /* 16 Task flags */
    UINT      error_mode;         /* 18 Error mode (see SetErrorMode)*/
    WORD      version;            /* 1a Expected Windows version */
    HANDLE    hInstance;          /* 1c Instance handle for task */
    HMODULE   hModule;            /* 1e Module handle */
    HQUEUE    hQueue;             /* 20 Selector of task queue */
    HTASK     hParent;            /* 22 Selector of TDB of parent */
    WORD      signal_flags;       /* 24 Flags for signal handler */
    FARPROC   sighandler;         /* 26 Signal handler */
    FARPROC   userhandler;        /* 2a USER signal handler */
    FARPROC   discardhandler;     /* 2e Handler for GlobalNotify() */
    FARPROC   int0;               /* 32 int 0 (divide by 0) handler */
    FARPROC   int2;               /* 36 int 2 (NMI) handler */
    FARPROC   int4;               /* 3a int 4 (INTO) handler */
    FARPROC   int6;               /* 3e int 6 (invalid opc) handler */
    FARPROC   int7;               /* 42 int 7 (coprocessor) handler */
    FARPROC   int3e;              /* 46 int 3e (80x87 emu) handler */
    FARPROC   int75;              /* 4a int 75 (80x87 error) handler */
    DWORD     compat_flags;       /* 4e Compatibility flags */
    BYTE      unused4[2];         /* 52 */
    DWORD unk;//struct _TEB *teb;             /* 54 Pointer to thread database */
    BYTE      unused5[8];         /* 58 */
    HANDLE    hPDB;               /* 60 Selector of PDB (i.e. PSP) */
    DWORD    dta;                /* 62 Current DTA */
    BYTE      curdrive;           /* 66 Current drive */
    char      curdir[65];         /* 67 Current directory */
    WORD      nCmdShow;           /* a8 cmdShow parameter to WinMain */
    HTASK     hYieldTo;           /* aa Next task to schedule */
    DWORD     dlls_to_init;       /* ac Ptr to DLLs to initialize */
    HANDLE    hCSAlias;           /* b0 Code segment for this TDB */
    WORD      thunks[8*4];        /* b2 Make proc instance thunks */
    char      module_name[8];     /* f2 Module name for task */
    WORD      magic;              /* fa TDB signature */
    HANDLE    hEvent;             /* fc scheduler event handle */ //?? @todo seems not in original TDB
	WORD padding;
    PDB       pdb;                /* 100 PDB for this task */
} TDB;

/* TDB flags */
#define TDBF_WINOLDAP   0x0001
#define TDBF_OS2APP     0x0008
#define TDBF_WIN32      0x0010

/* BURGERMASTER */

typedef struct _BURGERMASTER{
                BOOL bCheck;
                BOOL bFreeze;
                WORD wEntries;
        #ifdef MODE286
                WORD nwFirst;
                WORD nwLast;
        #else
                DWORD nwFirst;
                DWORD nwLast;
        #endif
                BYTE byComp;
                BYTE byDestrLev;
        #ifdef MODE286
                WORD nwDestrBytes;
                HANDLE hTable;
                HANDLE hFree;
                WORD wDelta;
                WORD pExpand;
                WORD pStats;
        #else
                DWORD nwDestrBytes;
                WORD reserved[5];
        #endif
                WORD wLRUAcess;
        #ifdef MODE286
                WORD wLRUChain;
        #else
                DWORD dwLRUChain;
        #endif
                WORD wLRUCount;
        #ifdef MODE286
                WORD nwDestrPara;
                WORD nwDestrChain;
        #else
                DWORD nwDestrPara;
                DWORD nwDestrChain;
        #endif
                WORD wFree;
} BURGERMASTER;
#if 0
Поля:
bCheck  Если TRUE, то структура не разрушена.
bFreeze Если TRUE, то структура не может быть изменена или сжата.
wEntries        Количество элементов в списке объектов глобального хипа (GLOBAL_ARENA).
nwFirst Селектор (для расширенного режима - смещение внутри сегмента BURGERMASTER) первого объекта в списке с обратной связью.
nwLast  Селектор (для расширенного режима - смещение внутри сегмента BURGERMASTER) последнего объекта в списке.
byComp  Уровень сжатия структуры.
byDestrLev      Текущий уровень разрушения структуры.
nwDestrBytes    Текущее число недействительных байт таблицы.
hTable  Дескриптор заголовка списка таблиц дескрипторов.
hFree   Дескриптор заголовка списка таблиц свободных дескрипторов.
wDelta  Количество дескрипторов, выделяемых по каждому запросу.
pExpand Адрес для процедур обработки дескрипторов.
pStats  Адрес статистических таблиц.
wLRUAccess      Доступ к списку LRU на уровне прерываний.
wLRUChain       Адрес первого дескриптора в списке LRU.
dwLRUChain      Смещение внутри BURGERMASTER первого дескриптора из списка LRU.
wLRUCount       Количество элементов в списке LRU.
nwDestrPara     Количество параграфов для разрушимого кода.
nwDestrReserve  Ограничитель разрушимого кода.
wFree   Количество свободных блоков.

Комментарии:
Структура определяет состояние глобальной памяти (хипа).
Основные (для программиста-практика) поля этой структуры - nwFirst и nwLast, 
адресующие начало и конец двунаправленного списка информационных блоков объектов
 глобального хипа (GLOBAL_ARENA), соответствующих MCB DOS.
Селектор этой структуры возвращается функцией GlobalMasterHandle(). 

По возможности рекомендуется вместо этой структуры использовать соответствующие средства ToolHelp.
#endif


/* THHOOK Kernel Data Structure */
typedef struct _THHOOK
{
    HANDLE   hGlobalHeap;         /* 00 (handle BURGERMASTER) */
    WORD     pGlobalHeap;         /* 02 (selector BURGERMASTER) */
    HMODULE  hExeHead;            /* 04 hFirstModule */
    HMODULE  hExeSweep;           /* 06 (unused) */
    HANDLE   TopPDB;              /* 08 (handle of KERNEL PDB) */
    HANDLE   HeadPDB;             /* 0A (first PDB in list) */
    HANDLE   TopSizePDB;          /* 0C (unused) */
    HTASK    HeadTDB;             /* 0E hFirstTask */
    HTASK    CurTDB;              /* 10 hCurrentTask */
    HTASK    LoadTDB;             /* 12 (unused) */
    HTASK    LockTDB;             /* 14 hLockedTask */
} THHOOK;

/* this structure is always located at offset 0 of the DGROUP segment */
typedef struct tagINSTANCEDATA
{
    WORD null;        /* Always 0 */
    WORD old_sp;      /* Stack pointer; used by SwitchTaskTo() */
    WORD old_ss;
    WORD heap;        /* near Pointer to the local heap information (if any) */
    WORD atomtable;   /* near Pointer to the local atom table (if any) */
    WORD stacktop;    /* Top of the stack */
    WORD stackmin;    /* Lowest stack address used so far */
    WORD stackbottom; /* Bottom of the stack */
} INSTANCEDATA, * PINSTANCEDATA, FAR * LPINSTANCEDATA;

/* In-memory module structure. See 'Windows Internals' p. 219 */
typedef struct tagNE_MODULE
{
    WORD      ne_magic;         /* 00 'NE' signature */
    WORD      count;            /* 02 Usage count (ne_ver/ne_rev on disk) */
    WORD      ne_enttab;        /* 04 Near ptr to entry table */
    HMODULE next;             /* 06 Selector to next module (ne_cbenttab on disk) */
    WORD      dgroup_entry;     /* 08 Near ptr to segment entry for DGROUP (ne_crc on disk) */
    WORD      fileinfo;         /* 0a Near ptr to file info (OFSTRUCT) (ne_crc on disk) */
    WORD      ne_flags;         /* 0c Module flags */
    WORD      ne_autodata;      /* 0e Logical segment for DGROUP */
    WORD      ne_heap;          /* 10 Initial heap size */
    WORD      ne_stack;         /* 12 Initial stack size */
    DWORD     ne_csip;          /* 14 Initial cs:ip */
    DWORD     ne_sssp;          /* 18 Initial ss:sp */
    WORD      ne_cseg;          /* 1c Number of segments in segment table */
    WORD      ne_cmod;          /* 1e Number of module references */
    WORD      ne_cbnrestab;     /* 20 Size of non-resident names table */
    WORD      ne_segtab;        /* 22 Near ptr to segment table */
    WORD      ne_rsrctab;       /* 24 Near ptr to resource table */
    WORD      ne_restab;        /* 26 Near ptr to resident names table */
    WORD      ne_modtab;        /* 28 Near ptr to module reference table */
    WORD      ne_imptab;        /* 2a Near ptr to imported names table */
    DWORD     ne_nrestab;       /* 2c File offset of non-resident names table */
    WORD      ne_cmovent;       /* 30 Number of moveable entries in entry table*/
    WORD      ne_align;         /* 32 Alignment shift count */
    WORD      ne_cres;          /* 34 # of resource segments */
    BYTE      ne_exetyp;        /* 36 Operating system flags */
    BYTE      ne_flagsothers;   /* 37 Misc. flags */
    HANDLE  dlls_to_init;     /* 38 List of DLLs to initialize (ne_pretthunks on disk) */
    HANDLE  nrname_handle;    /* 3a Handle to non-resident name table (ne_psegrefbytes on disk) */
    WORD      ne_swaparea;      /* 3c Min. swap area size */
    WORD      ne_expver;        /* 3e Expected Windows version */
    /* From here, these are extra fields not present in normal Windows */
//    HMODULE   module32;         /* PE module handle for Win32 modules */
//    HMODULE   owner32;          /* PE module containing this one for 16-bit builtins */
//    HMODULE16 self;             /* Handle for this module */
//    WORD      self_loading_sel; /* Selector used for self-loading apps. */
//    LPVOID    rsrc32_map;       /* HRSRC 16->32 map (for 32-bit modules) */
//    LPCVOID   mapping;          /* mapping of the binary file */
//    SIZE_T    mapping_size;     /* size of the file mapping */
} NE_MODULE, * PNE_MODULE, FAR * LPNE_MODULE;

typedef HMODULE * PHMODULE, FAR * LPHMODULE;

#define NE_MODULE_NAME(pModule) \
    (((OFSTRUCT FAR *)((char FAR *)(pModule) + (pModule)->fileinfo))->szPathName)

  /* In-memory segment table */
typedef struct
{
    WORD      filepos;   /* Position in file, in sectors */
    WORD      size;      /* Segment size on disk */
    WORD      flags;     /* Segment flags */
    WORD      minsize;   /* Min. size of segment in memory */
    HANDLE    hSeg;      /* Selector or handle (selector - 1) of segment in memory */
} SEGTABLEENTRY;

#define IMAGE_OS2_SIGNATURE    0x454E     /* NE   */

#define NE_SEG_TABLE(pModule) \
    ((SEGTABLEENTRY *)((char *)(pModule) + (pModule)->ne_segtab))

/*
 * NE Header FORMAT FLAGS
 */
#define NE_FFLAGS_SINGLEDATA    0x0001
#define NE_FFLAGS_MULTIPLEDATA  0x0002
#define NE_FFLAGS_WIN32         0x0010
#define NE_FFLAGS_BUILTIN       0x0020  /* Wine built-in module */
#define NE_FFLAGS_FRAMEBUF      0x0100  /* OS/2 fullscreen app */
#define NE_FFLAGS_CONSOLE       0x0200  /* OS/2 console app */
#define NE_FFLAGS_GUI           0x0300  /* right, (NE_FFLAGS_FRAMEBUF | NE_FFLAGS_CONSOLE) */
#define NE_FFLAGS_SELFLOAD      0x0800
#define NE_FFLAGS_LINKERROR     0x2000
#define NE_FFLAGS_CALLWEP       0x4000
#define NE_FFLAGS_LIBMODULE     0x8000


/*#define NE_READ_DATA(pModule,buffer,offset,size) \
    (((offset)+(size) <= pModule->mapping_size) ? \
     (memcpy( buffer, (const char *)pModule->mapping + (offset), (size) ), TRUE) : FALSE)
*/
/*
 * Resource table structures.
 */
typedef struct tagNE_NAMEINFO
{
    WORD     offset;
    WORD     length;
    WORD     flags;
    WORD     id;
    HANDLE   handle;
    WORD     usage;
} NE_NAMEINFO, * PNE_NAMEINFO, FAR * LPNE_NAMEINFO;

typedef struct tagNE_TYPEINFO
{
    WORD        type_id;   /* Type identifier */
    WORD        count;     /* Number of resources of this type */
    FARPROC   resloader; /* SetResourceHandler() */
    /*
     * Name info array.
     */
} NE_TYPEINFO, * PNE_TYPEINFO, FAR * LPNE_TYPEINFO;

#define NE_SEGFLAGS_LOADED      0x0004

/* Layout of a handle entry table
 *
 * WORD                     count of entries
 * LOCALHANDLEENTRY[count]  entries
 * WORD                     near ptr to next table
 */
typedef struct tagLOCALHANDLEENTRY
{
    WORD addr;                /* Address of the MOVEABLE block */
    BYTE flags;               /* Flags for this block */
    BYTE lock;                /* Lock count */
} LOCALHANDLEENTRY, * PLOCALHANDLEENTRY, FAR * LPLOCALHANDLEENTRY;
