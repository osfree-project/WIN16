#include <conio.h> /* this is where Open Watcom hides the outp() etc. functions */
#include <unistd.h> /* this is where Open Watcom hides the outp() etc. functions */

extern unsigned outpw(unsigned, unsigned);
#pragma aux outpw = \
		"out dx, ax" \
		parm  [dx] [ax] \
                value [ax];

extern unsigned outp(unsigned, unsigned);
#pragma aux outp = \
		"out dx, al" \
		parm  [dx] [al] \
                value [ax];

extern unsigned inpw(unsigned);
#pragma aux inpw = \
		"in ax, dx" \
		parm  [dx] \
                value [ax];

#define DOSBOXID_VAR

uint16_t DOSBOXID_VAR dosbox_id_baseio = 0x28U;	// Default ports 0x28 - 0x2B

# define DOSBOX_IDPORT(x)                                (dosbox_id_baseio+(x))

#define DOSBOX_ID_INDEX                                 (0U) /* R/W */
#define DOSBOX_ID_DATA                                  (1U) /* R/W */
#define DOSBOX_ID_COMMAND                               (2U) /*   W */

/* DOSBOX_ID_COMMAND */
#define DOSBOX_ID_CMD_RESET_LATCH                       (0x00U)
#define DOSBOX_ID_CMD_FLUSH_WRITE                       (0x01U)
#define DOSBOX_ID_CMD_RESET_INTERFACE                   (0xFFU)

/* DOSBOX_ID_DATA after reset */
#define DOSBOX_ID_RESET_DATA_CODE                       (0x0D05B0C5UL)
#define DOSBOX_ID_RESET_INDEX_CODE                      (0xAA55BB66UL)

/* DOSBOX_ID_INDEX */
#define DOSBOX_ID_REG_DEBUG_OUT                         (0x0000DEB0UL)

#define DOSBOX_ID_REG_IDENTIFY                          (0x00000000UL)

/* return value of DOSBOX_ID_REG_IDENTIFY */
#define DOSBOX_ID_IDENTIFICATION                        (0xD05B0740UL)

static inline void dosbox_id_reset_interface() {
	outp(DOSBOX_IDPORT(DOSBOX_ID_COMMAND),DOSBOX_ID_CMD_RESET_INTERFACE);
}

static inline void dosbox_id_reset_latch() {
	outp(DOSBOX_IDPORT(DOSBOX_ID_COMMAND),DOSBOX_ID_CMD_RESET_LATCH);
}

static inline void dosbox_id_write_data_nrl_u8(const unsigned char c) {
	outp(DOSBOX_IDPORT(DOSBOX_ID_DATA),c);
}

static inline void dosbox_id_flush_write() {
	outp(DOSBOX_IDPORT(DOSBOX_ID_COMMAND),DOSBOX_ID_CMD_FLUSH_WRITE);
}
uint32_t dosbox_id_read_data_nrl() {
	uint32_t r;

#if TARGET_MSDOS == 32
	r  = (uint32_t)inpd(DOSBOX_IDPORT(DOSBOX_ID_DATA));
#else
	r  = (uint32_t)inpw(DOSBOX_IDPORT(DOSBOX_ID_DATA));
	r |= (uint32_t)inpw(DOSBOX_IDPORT(DOSBOX_ID_DATA)) << (uint32_t)16UL;
#endif

	return r;
}
uint32_t dosbox_id_read_data() {
	dosbox_id_reset_latch();
	return dosbox_id_read_data_nrl();
}

uint32_t dosbox_id_read_regsel() {
	uint32_t r;

	dosbox_id_reset_latch();

#if TARGET_MSDOS == 32
	r  = (uint32_t)inpd(DOSBOX_IDPORT(DOSBOX_ID_INDEX));
#else
	r  = (uint32_t)inpw(DOSBOX_IDPORT(DOSBOX_ID_INDEX));
	r |= (uint32_t)inpw(DOSBOX_IDPORT(DOSBOX_ID_INDEX)) << (uint32_t)16UL;
#endif

	return r;
}

int dosbox_id_reset() {
	uint32_t t1,t2;

	/* on reset, data should return DOSBOX_ID_RESET_DATA_CODE and index should return DOSBOX_ID_RESET_INDEX_CODE */
	dosbox_id_reset_interface();
	t1 = dosbox_id_read_data();
	t2 = dosbox_id_read_regsel();
	if (t1 != DOSBOX_ID_RESET_DATA_CODE || t2 != DOSBOX_ID_RESET_INDEX_CODE) return 0;
	return 1;
}

void dosbox_id_write_regsel(const uint32_t reg) {
	dosbox_id_reset_latch();

#if TARGET_MSDOS == 32
	outpd(DOSBOX_IDPORT(DOSBOX_ID_INDEX),reg);
#else
	outpw(DOSBOX_IDPORT(DOSBOX_ID_INDEX),(uint16_t)reg);
	outpw(DOSBOX_IDPORT(DOSBOX_ID_INDEX),(uint16_t)(reg >> 16UL));
#endif
}

uint32_t dosbox_id_read_identification() {
	/* now read the identify register */
	dosbox_id_write_regsel(DOSBOX_ID_REG_IDENTIFY);
	return dosbox_id_read_data();
}

int probe_dosbox_id() {
	uint32_t t;

	if (!dosbox_id_reset()) return 0;

	t = dosbox_id_read_identification();
	if (t != DOSBOX_ID_IDENTIFICATION) return 0;

	return 1;
}

void dosbox_id_debug_message(const char *str) {
	unsigned char c;

	dosbox_id_write_regsel(DOSBOX_ID_REG_DEBUG_OUT);
	dosbox_id_reset_latch();
	while ((c=((unsigned char)(*str++))) != 0U) dosbox_id_write_data_nrl_u8(c);
	dosbox_id_flush_write();
}
