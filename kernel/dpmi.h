/* DPMI Library */

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

