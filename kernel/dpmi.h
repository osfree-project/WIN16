/* DPMI Library */

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
    parm [bx] [es di] \
    value [ax] \
    modify []
