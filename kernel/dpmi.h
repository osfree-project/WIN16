/* DPMI Library */

#pragma pack(push,1)
typedef struct _LDT_ENTRY {
    WORD	LimitLow;
    WORD	BaseLow;
    union {
        struct {
            BYTE    BaseMid;
            BYTE    Flags1;
            BYTE    Flags2;
            BYTE    BaseHi;
        } Bytes;
        struct {
            unsigned    BaseMid: 8;
            unsigned    Type : 5;
            unsigned    Dpl : 2;
            unsigned    Pres : 1;
            unsigned    LimitHi : 4;
            unsigned    Sys : 1;
            unsigned    Reserved_0 : 1;
            unsigned    Default_Big : 1;
            unsigned    Granularity : 1;
            unsigned    BaseHi : 8;
        } Bits;
    } HighWord;
} LDT_ENTRY;
#pragma pack(pop)

extern int DPMI_AllocDesc(unsigned int);
#pragma aux DPMI_AllocDesc        = \
        "mov    ax,0000h"          \
	"int    31h"\
	"jnc	exit"\
	"xor	ax,ax"\
	"exit:"\
	modify [] \
	parm [cx] \
        value [ax];

extern int DPMI_FreeDesc(unsigned int);
#pragma aux DPMI_FreeDesc        = \
        "mov    ax,0001h"          \
	"int    31h"\
	"jnc	exit"\
	"xor	ax,ax"\
	"exit:"\
	modify [] \
	parm [bx] \
        value [ax];

extern unsigned long DPMI_GetBase(unsigned int);
#pragma aux DPMI_GetBase        = \
        "mov    ax,0006h"          \
	"int    31h"\
	"jnc	exit"\
	"xor	cx,cx"\
	"xor	dx,dx"\
	"exit:"\
	modify [ax] \
	parm [bx] \
        value [cx dx];

extern int DPMI_SetBase(unsigned int, unsigned long);
#pragma aux DPMI_SetBase        = \
        "mov    ax,0007h"          \
	"int    31h"\
	"sbb	ax,ax"\
	parm [bx] [cx bx] \
        value [ax];

extern int DPMI_SetLimit(unsigned int, unsigned long);
#pragma aux DPMI_SetLimit        = \
        "mov    ax,0008h"          \
	"int    31h"\
	"sbb	ax,ax"\
	parm [bx] [cx bx] \
        value [ax];



extern int DPMI_CreateCSAlias(unsigned int);
#pragma aux DPMI_CreateCSAlias        = \
        "mov    ax,0008h"          \
	"int    31h"\
	"jnc	exit"\
	"xor	ax,ax"\
	"exit:"\
	parm [bx] \
        value [ax];

extern int DPMI_GetDescriptor(unsigned int, LDT_ENTRY far *);
#pragma aux DPMI_GetDescriptor        = \
        "mov    ax,000Bh"          \
	"int    31h"\
	"sbb	ax,ax"\
	modify [] \
	parm [bx] [es di] \
        value [ax];

extern int DPMI_SetDescriptor(unsigned int, LDT_ENTRY far *);
#pragma aux DPMI_SetDescriptor        = \
        "mov  ax,000Bh"   \
        "int 31h"       \
        "sbb  ax,ax"    \
    parm [bx] [di es] \
    value [ax] \
    modify []

#pragma pack(push,1)
typedef struct {
	char major_version;
	char minor_version;
	int flags;
	char processor_type;
	int host_mem;
	void(far * switchentry)(void);
} init_info;
#pragma pack(pop)

extern void far * DPMI_Init( init_info far * );
#pragma aux DPMI_Init = \
		"push ds" \
		"push cx" \
		"pop ds" \
		"push di" \
		"mov ax,1687h" \
		"int 2fh" \
		"or ax, ax" \
		"pop ax" \
		"jnz err" \
		"push ax" \
		"mov ax, si" \
		"pop si" \
		"mov byte ptr ds:[si],dh" \
		"mov byte ptr ds:[si+1],dl" \
		"mov word ptr ds:[si+2],bx" \
		"mov byte ptr ds:[si+4],cl" \
		"mov word ptr ds:[si+5],ax" \
		"mov word ptr ds:[si+7],di" \
		"mov word ptr ds:[si+9],es" \
		"mov cx, es"\
		"jmp exit" \
		"err:" \
		"xor ax, ax" \
		"mov cx, ax" \
		"mov di, ax" \
		"exit: " \
		"pop ds" \
		parm[cx di] modify[ax bx cl dx si] value [cx di] ;

extern void far * DPMI_VendorEntry(char far * szVendorStr);
#if 0
#pragma aux DPMI_VendorEntry = \
        "push ds"       \
        "push es"       \
        "mov  ds,cx"   \
        "mov  cx,es"   \
	"mov	ax, 0a00h" \
	"int	31h" \
	"jnc L1"\
        "xor  cx,cx"  \
        "xor  di,di"  \
    "L1: pop  es"       \
        "pop  ds"       \
    parm [cx si] \
    value [cx di] \
    modify [ax cx]
#else
#pragma aux DPMI_VendorEntry = \
        "push ds"       \
        "push es"       \
        "mov  ds,cx"   \
        "mov  cx,es"   \
	"mov	ax, 168ah" \
	"int	2fh" \ 
        "cmp al,8ah" \
	"jne short L1"  \
        "xor  cx,cx"  \
        "xor  di,di"  \
    "L1: pop  es"       \
        "pop  ds"       \
    parm [cx si] \
    value [cx di] \
    modify [ax cx]
#endif

extern char DPMI_GetCPU();
// 486 by default
#pragma aux DPMI_GetCPU        = \
        "mov    ax,0400h"          \
	"int    31h"\
	"jnc	exit"\
	"mov	cl,4"\	
	"exit:"\
        value [cl];
