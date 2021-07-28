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
    .global ddrcfg_override_start

/**************************************************************************************
   A P P L I C A T I O N    E N T R Y    P O I N T
**************************************************************************************/
LEAF(__app_entry)

    b       store_regs
    nop
    nop
    nop

    // Offset 0x10 is used for overriding DDR configuration
ddrcfg_override_start:
    nop
    .org 128

store_regs:
    addiu   sp, sp, -88
    sw      ra, 0(sp)
    sw      gp, 8(sp)
    sw      s0, 16(sp)
    sw      s1, 24(sp)
    sw      s2, 32(sp)
    sw      s3, 40(sp)
    sw      s4, 48(sp)
    sw      s5, 56(sp)
    sw      s6, 64(sp)
    sw      s7, 72(sp)
    sw      s8, 80(sp)
    dla     gp, _gp

    // Clear bss
zero_bss:
    li      s1, 0xffffffff
    dla      a1, _fbss
    dla      a2, _end
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

#ifdef CONFIG_BOOTMODE_JTAG
loop:
    // Software breakpoint
    sdbbp   0
    bc      loop
#endif

    // Jump to previously stored return address
    lw      ra, 0(sp)
    lw      gp, 8(sp)
    lw      s0, 16(sp)
    lw      s1, 24(sp)
    lw      s2, 32(sp)
    lw      s3, 40(sp)
    lw      s4, 48(sp)
    lw      s5, 56(sp)
    lw      s6, 64(sp)
    lw      s7, 72(sp)
    lw      s8, 80(sp)
    addiu   sp, sp, 88
    jrc     ra

END(__app_entry)
