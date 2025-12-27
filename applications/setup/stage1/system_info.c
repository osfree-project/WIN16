#include <dos.h>
#include "system_info.h"
#include "log.h"

int detect_cpu_type() {
    unsigned short flags1, flags2;
    
    _asm {
        pushf
        pop ax
        mov flags1, ax
        xor ax, 0x4000
        push ax
        popf
        pushf
        pop ax
        mov flags2, ax
        push flags1
        popf
    }
    
    if ((flags1 ^ flags2) & 0x4000) {
        /* 286 or better detected */
        unsigned int result;
        
        _asm {
            ; Try to toggle AC bit in EFLAGS (bit 18) - only works on 386+
            pushf
            pushf
            pop ax
            mov bx, ax
            xor ax, 4000h   ; Try to change bit 18 (AC flag)
            push ax
            popf
            pushf
            pop ax
            popf
            cmp ax, bx
            je not_386
            mov result, 2   ; 386 detected
            jmp done
        not_386:
            mov result, 1   ; 286 detected  
        done:
        }
        
        if (result == 2) {
            log_message("CPU detected: 386");
            return 2; /* CPU_386 */
        } else {
            log_message("CPU detected: 286");
            return 1; /* CPU_286 */
        }
    }
    
    log_message("CPU detected: 8086");
    return 0; /* CPU_8086 */
}

long detect_memory() {
    unsigned short extended_memory = 0;
    _asm {
        mov ah, 0x88
        int 0x15
        mov extended_memory, ax
    }
    return (long)extended_memory * 1024L;
}
