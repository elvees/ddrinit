/*
 *  start.S
 *
 *  Created on: Dec 13, 2018
 *  Author: Sondrel
 *  Boot code of bootrom application
*/

#include <regdef.h>
#include <mips/asm.h>
#include <config.h>

    .set    noreorder           // Don't allow the assembler to reorder instructions.
    .set    noat                // Don't allow the assembler to use r1(at) for synthetic instr.

/**************************************************************************************
   A P P L I C A T I O N    E N T R Y    P O I N T
**************************************************************************************/
LEAF(__app_entry)
    // Store return address
    addiu   sp, sp, -16
    sw      ra, 0(sp)
    sw      a1, 4(sp)
    sw      a2, 8(sp)
    sw      s1, 12(sp)
    dla     gp, _gp

    // Clear bss
zero_bss:
    li      s1, 0xffffffff
    dla      a1, _fbss
    ins     a1, s1, 29, 1
    dla      a2, _end
    ins     a2, s1, 29, 1
    beq     a1, a2, bss_done
    nop
next_bss_word:
    sw      zero, 0(a1)
    addiu   a1, 4
    bne     a1, a2, next_bss_word
    nop
bss_done:

    // Jump to main()
    jal     main
    nop

#ifdef CONFIG_BOOTMODE_NOSTART
loop:
    wait
    b loop
#endif

    // Jump to previously stored return address
    lw      ra, 0(sp)
    lw      a1, 4(sp)
    lw      a2, 8(sp)
    lw      s1, 12(sp)
    addiu   sp, sp, 16
    jr      ra

END(__app_entry)
