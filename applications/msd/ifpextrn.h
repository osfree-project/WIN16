/* IFPEXTRN.H – Interface to assembly routines (from ifpextrn.pas) */
#ifndef IFPEXTRN_H
#define IFPEXTRN_H

#include <dos.h>

typedef struct {
    unsigned char  cpu_type;
    unsigned int   MSW;
    unsigned char  GDT[6];
    unsigned char  IDT[6];
    unsigned char  intflag;
    unsigned char  ndp_type;
    unsigned int   ndp_cw;
    unsigned char  weitek;
    char           test_type;
} cpu_info_t;

void CPUID(cpu_info_t *a);
unsigned int diskread(unsigned char drive, unsigned long starting_sector,
                      unsigned int number_of_sectors, void __far *buffer);
void longcall(unsigned long addr, union REGS *regs);
unsigned char ATIinfo(unsigned char data_in, unsigned int reg);
void AltIntr(unsigned char intno, union REGS *regs);
void AltMsDos(union REGS *regs);
unsigned char CTICK(void);
unsigned char TsengCK(void);
unsigned char ZyMOSCK(void);
unsigned char CirrusCK(void);
unsigned char bugtst(void);

#endif
