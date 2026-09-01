/**
 * @file dpmi.h
 * @brief DPMI (DOS Protected Mode Interface) Library
 *
 * This header provides functions and types for DPMI services.
 */

/**
 * @struct _LDT_ENTRY
 * @brief 8-byte Local Descriptor Table (LDT) descriptor.
 *
 * The descriptor consists of limit, base, and access flags fields.
 * The fields can be interpreted either as bytes (Bytes) or as bit fields (Bits),
 * depending on the context.
 *
 * Memory layout (bytes 0..7):
 * @code
 * Bytes:  [0]      [1]      [2]      [3]      [4]      [5]      [6]             [7]
 * Fields: | LimitLow       | BaseLow        | BaseMid | Flags1  | Flags2        | BaseHi  |
 * Bits:   | 15..0 of limit | 15..0 of base  | 23..16  |T T T T T|L L L L S R D G| 31..24  |
 *                                           | of base |D D P    |               | of base |
 * @endcode
 *
 * Detailed field descriptions:
 * - LimitLow   : low 16 bits of segment limit
 * - BaseLow    : low 16 bits of base address
 * - BaseMid    : bits 16–23 of base address
 * - Flags1     : access byte (Type[4:0], DPL[6:5], P[7])
 * - Flags2     : LimitHi[3:0], Sys, Reserved, Default_Big, Granularity
 * - BaseHi     : bits 24–31 of base address
 */

#pragma pack(push,1)
typedef struct _LDT_ENTRY {
    WORD    LimitLow;   ///< Low word of segment limit (bits 0–15).
    WORD    BaseLow;    ///< Low word of segment base (bits 0–15).
    union {
        struct {
            BYTE    BaseMid;    ///< Middle byte of base (bits 16–23).
            BYTE    Flags1;     ///< Access flags byte.
            BYTE    Flags2;     ///< Additional flags byte.
            BYTE    BaseHi;     ///< High byte of base (bits 24–31).
        } Bytes;    ///< Byte-wise representation of the high word.
        struct {
            unsigned    BaseMid: 8;          ///< Base address bits 16–23.
            unsigned    Type : 5;            ///< Segment type (code/data, access, etc.).
            unsigned    Dpl : 2;             ///< Descriptor Privilege Level (DPL).
            unsigned    Pres : 1;            ///< Segment present bit.
            unsigned    LimitHi : 4;         ///< High 4 bits of limit (bits 16–19).
            unsigned    Sys : 1;             ///< System descriptor bit (0 = system).
            unsigned    Reserved_0 : 1;      ///< Reserved, must be 0.
            unsigned    Default_Big : 1;     ///< Default operand size (0=16-bit, 1=32-bit).
            unsigned    Granularity : 1;     ///< Limit granularity (0=bytes, 1=4KB pages).
            unsigned    BaseHi : 8;          ///< Base address bits 24–31.
        } Bits;    ///< Bit-field representation of the high word.
    } HighWord;     ///< Upper 4 bytes of the descriptor (union of bytes and bits).
} LDT_ENTRY;
#pragma pack(pop)

/**
 * @brief Allocates one or more descriptors in the LDT.
 *
 * Uses DPMI service INT 31h, AX=0000h.
 *
 * @param[in] cx Number of descriptors to allocate (usually 1).
 * @return Descriptor (selector) on success, 0 on error.
 *
 * @note A return value of 0 indicates an error because selector 0 is reserved.
 */

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

/**
 * @brief Frees an LDT descriptor.
 *
 * Uses DPMI service INT 31h, AX=0001h.
 *
 * @param[in] bx Selector of the descriptor to free.
 * @return 0 on success, -1 on error.
 */
extern int DPMI_FreeDesc(unsigned int);
#pragma aux DPMI_FreeDesc        = \
        "mov    ax,0001h"          \
	"int    31h"\
	"sbb	ax,ax"\
	modify [] \
	parm [bx] \
        value [ax];

/**
 * @brief Gets the 32-bit base address of a segment.
 *
 * Uses DPMI service INT 31h, AX=0006h.
 *
 * @param[in] bx Segment selector.
 * @return 32-bit base address in CX:DX; 0 on error.
 */
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

/**
 * @brief Sets the 32-bit base address of a segment.
 *
 * Uses DPMI service INT 31h, AX=0007h.
 *
 * @param[in] bx Segment selector.
 * @param[in] cx_dx New base address (passed in CX:DX).
 * @return 0 on success, -1 on error (CF set, AX = -1).
 */
extern int DPMI_SetBase(unsigned int, unsigned long);
#pragma aux DPMI_SetBase        = \
        "mov    ax,0007h"          \
	"int    31h"\
	"sbb	ax,ax"\
	parm [bx] [cx dx] \
        value [ax];

/**
 * @brief Sets the 32-bit limit of a segment.
 *
 * Uses DPMI service INT 31h, AX=0008h.
 *
 * @param[in] bx Segment selector.
 * @param[in] cx_dx New limit (passed in CX:DX).
 * @return 0 on success, -1 on error.
 */
extern int DPMI_SetLimit(unsigned int, unsigned long);
#pragma aux DPMI_SetLimit        = \
        "mov    ax,0008h"          \
	"int    31h"\
	"sbb	ax,ax"\
	parm [bx] [cx dx] \
        value [ax];

/**
 * @brief Creates a code segment alias for a data segment.
 *
 * Uses DPMI service INT 31h, AX=000Ah.
 *
 * @param[in] bx Selector of the data segment for which to create an alias.
 * @return New selector (alias) on success, 0 on error.
 */
extern int DPMI_CreateCSAlias(unsigned int);
#pragma aux DPMI_CreateCSAlias        = \
        "mov    ax,000Ah"          \
	"int    31h"\
	"jnc	exit"\
	"xor	ax,ax"\
	"exit:"\
	parm [bx] \
        value [ax];

/**
 * @brief Copies the contents of an LDT descriptor into an LDT_ENTRY structure.
 *
 * Uses DPMI service INT 31h, AX=000Bh.
 *
 * @param[in] bx Selector of the descriptor.
 * @param[out] es_di Far pointer to an LDT_ENTRY structure to receive the descriptor.
 * @return 0 on success, -1 on error.
 */
extern int DPMI_GetDescriptor(unsigned int, LDT_ENTRY far *);
#pragma aux DPMI_GetDescriptor        = \
        "mov    ax,000Bh"          \
	"int    31h"\
	"sbb	ax,ax"\
	modify [] \
	parm [bx] [es di] \
        value [ax];

/**
 * @brief Writes the contents of an LDT_ENTRY structure into an LDT descriptor.
 *
 * Uses DPMI service INT 31h, AX=000Ch.
 *
 * @param[in] bx Selector of the descriptor.
 * @param[in] di_es Far pointer to an LDT_ENTRY structure containing the data to write.
 * @return 0 on success, -1 on error.
 */
extern int DPMI_SetDescriptor(unsigned int, LDT_ENTRY far *);
#pragma aux DPMI_SetDescriptor        = \
        "mov  ax,000Ch"   \
        "int 31h"       \
        "sbb  ax,ax"    \
    parm [bx] [di es] \
    value [ax] \
    modify []

/**
 * @struct init_info
 * @brief Structure filled during DPMI initialization (INT 2Fh, AX=1687h).
 *
 * Contains DPMI version, flags, processor type, available memory,
 * and the entry point for switching to protected mode.
 */
#pragma pack(push,1)
typedef struct {
    char major_version;     ///< DPMI major version.
    char minor_version;     ///< DPMI minor version.
    int flags;              ///< Capability flags (bit 0: 32-bit programs supported).
    char processor_type;    ///< Processor type (0=8086, 1=80186, 2=80286, 3=80386, etc.).
    int host_mem;           ///< Amount of host memory (in 16-KB units?).
    void(far * switchentry)(void); ///< Entry point for switching to protected mode.
} init_info;
#pragma pack(pop)

/**
 * @brief Initializes DPMI and fills the init_info structure.
 *
 * Calls multiplex interrupt INT 2Fh with AX=1687h to obtain the DPMI entry point
 * and system information. The supplied structure is then populated.
 *
 * @param[in] cx_di Far pointer to an init_info structure.
 * @return Far pointer to the DPMI entry point (for switching to protected mode)
 *         or NULL (0:0) on error.
 *
 * @note Parameter is passed in CX:DI; return value is also in CX:DI.
 */
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

/**
 * @brief Obtains the vendor-specific API entry point.
 *
 * Calls either DPMI INT 31h, AX=0A00h or multiplex INT 2Fh, AX=168Ah,
 * depending on the selected code path.
 *
 * @param[in] cx_si Far pointer to a vendor name string.
 * @return Far pointer to the vendor API entry point or NULL (0:0) on error.
 */
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

/**
 * @brief Determines the processor type via DPMI.
 *
 * Uses DPMI service INT 31h, AX=0400h.
 *
 * @return Processor number in CL (e.g., 3=80386, 4=80486, etc.).
 *         On error, 4 (486) is returned by default.
 */
extern char DPMI_GetCPU();
// 486 by default
#pragma aux DPMI_GetCPU        = \
        "mov    ax,0400h"          \
	"int    31h"\
	"jnc	exit"\
	"mov	cl,4"\	
	"exit:"\
        value [cl];

/**
 * @brief Frees a block of DOS memory (selector).
 *
 * Uses DPMI service INT 31h, AX=0101h.
 *
 * @param[in] dx Selector of the DOS memory block.
 * @return 0 on success, otherwise error code (value in DX).
 */
extern WORD DPMI_FreeDOSMem(WORD sel);
#pragma aux DPMI_FreeDOSMem = \
	"mov	ax, 0101H" \
	"int	31h" \
	"mov	ax,dx" \
	"jc		short L1" \
	"xor	ax,ax" \
	"L1:" \
	parm [dx] \
	value [ax];

/**
 * @brief Allocates a block of DPMI memory (function 0x0501).
 *
 * Uses DPMI service INT 31h, AX=0501h.
 *
 * @param[in] size Block size in bytes (passed in BX:CX).
 * @param[out] handle Pointer to a variable that will receive the block handle.
 * @param[out] linear Pointer to a variable that will receive the linear address.
 * @return 0 on success, otherwise error code.
 *
 * @note If handle or linear pointers are NULL, they are ignored.
 */
extern BOOL DPMI_AllocMem(DWORD size, DWORD *handle, DWORD *linear);
#pragma aux DPMI_AllocMem = \
    "mov ax, 0501h" \
    "int 31h" \
    "jnc ok" \
    "xor si, si" \
    "xor di, di" \
    "xor bx, bx" \
    "xor cx, cx" \
    "ok:" \
    parm [bx cx] [si di] [bx cx] \
    value [ax] \
    modify [];

/**
 * @brief Frees a block of DPMI memory.
 *
 * Uses DPMI service INT 31h, AX=0502h.
 *
 * @param[in] handle Memory block handle (passed in SI:DI).
 * @return 0 on success, -1 on error.
 */
extern BOOL DPMI_FreeMem(DWORD handle);
#pragma aux DPMI_FreeMem = \
    "mov ax, 0502h" \
    "int 31h" \
    "sbb ax, ax" \
    parm [si di] \
    value [ax] \
    modify [];

/**
 * @brief Gets the selector increment value (DPMI Int 31h AX=0003h).
 *
 * Uses DPMI service INT 31h, AX=0003h.
 *
 * @return Increment value (in bytes) to be added to a selector to get the next
 *         descriptor.
 */
extern WORD DPMI_GetIncrement(void);
#pragma aux DPMI_GetIncrement = \
    "mov ax, 0003h" \
    "int 31h" \
    parm [] \
    value [ax] \
    modify [];

/**
 * @brief Converts a real-mode segment to an LDT descriptor.
 *
 * Uses DPMI service INT 31h, AX=0002h.
 *
 * @param[in] segment Real-mode segment value.
 * @return Selector of the descriptor describing this segment; 0 on error.
 */
extern WORD DPMI_SegmentToDescriptor(WORD segment);
#pragma aux DPMI_SegmentToDescriptor = \
    "mov ax, 0002h" \
    "int 31h" \
    "jnc short exit" \
    "xor ax, ax" \
    "exit:" \
    parm [bx] \
    value [ax] \
    modify [];

/**
 * @brief Gets the number of free memory pages (DPMI Int 31h AX=0500h).
 *
 * Uses DPMI service INT 31h, AX=0500h.
 *
 * @return Number of free pages, or 0 on error.
 */
extern WORD DPMI_GetFreePages(void);
#pragma aux DPMI_GetFreePages = \
    "mov  ax, 0500h" \
    "int  31h" \
    "jc   short error" \
    "mov  bx, es:[di+10h]" \
    "jmp  short done" \
    "error:" \
    "xor  bx, bx" \
    "done:" \
    parm [] \
    value [bx] \
    modify [ax bx cx dx si di es];

