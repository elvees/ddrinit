/* SPDX-License-Identifier: GPL-2.0-or-later */
/* Copyright 2022 RnD Center "ELVEES", JSC */

#ifndef _MCOM03_ASM_H
#define _MCOM03_ASM_H

	.set  CONTEXT_R1, 4
	.set  CONTEXT_R2, 5
	.set  CONTEXT_R3, 6
	.set  CONTEXT_R4, 7
	.set  CONTEXT_R5, 8
	.set  CONTEXT_R6, 9
	.set  CONTEXT_R7, 10
	.set  CONTEXT_R8, 11
	.set  CONTEXT_R9, 12
	.set  CONTEXT_R10, 13
	.set  CONTEXT_R11, 14
	.set  CONTEXT_R12, 15
	.set  CONTEXT_R13, 16
	.set  CONTEXT_R14, 17
	.set  CONTEXT_R15, 18
	.set  CONTEXT_R16, 19
	.set  CONTEXT_R17, 20
	.set  CONTEXT_R18, 21
	.set  CONTEXT_R19, 22
	.set  CONTEXT_R20, 23
	.set  CONTEXT_R21, 24
	.set  CONTEXT_R22, 25
	.set  CONTEXT_R23, 26
	.set  CONTEXT_R24, 27
	.set  CONTEXT_R25, 28
	.set  CONTEXT_GP, 29
	.set  CONTEXT_FP, 30
	.set  CONTEXT_RA, 31
	.set  CONTEXT_LO, 32
	.set  CONTEXT_HI, 33
	.set  CONTEXT_STATUS, 34
	.set  CONTEXT_PC, 35
	.set  CONTEXT_F0, 36
	.set  CONTEXT_F1, 37
	.set  CONTEXT_F2, 38
	.set  CONTEXT_F3, 39
	.set  CONTEXT_F4, 40
	.set  CONTEXT_F5, 41
	.set  CONTEXT_F6, 42
	.set  CONTEXT_F7, 43
	.set  CONTEXT_F8, 44
	.set  CONTEXT_F9, 45
	.set  CONTEXT_F10, 46
	.set  CONTEXT_F11, 47
	.set  CONTEXT_F12, 48
	.set  CONTEXT_F13, 49
	.set  CONTEXT_F14, 50
	.set  CONTEXT_F15, 51
	.set  CONTEXT_F16, 52
	.set  CONTEXT_F17, 53
	.set  CONTEXT_F18, 54
	.set  CONTEXT_F19, 55
	.set  CONTEXT_F20, 56
	.set  CONTEXT_F21, 57
	.set  CONTEXT_F22, 58
	.set  CONTEXT_F23, 59
	.set  CONTEXT_F24, 60
	.set  CONTEXT_F25, 61
	.set  CONTEXT_F26, 62
	.set  CONTEXT_F27, 63
	.set  CONTEXT_F28, 64
	.set  CONTEXT_F29, 65
	.set  CONTEXT_F30, 66
	.set  CONTEXT_F31, 67
	.set  CONTEXT_FCSR, 68
	.set  CONTEXT_FLAG_HAS_FPU, 69
	.set  CONTEXT_FLAG_CRITICAL_NESTING, 70
	.set  CONTEXT_FLAG_INTERRUPT_NESTING, 71

	.set CONTEXT_WORDS, 72

#define zero 0
#define v0 2
#define v1 3
#define a0 4
#define a1 5
#define a2 6
#define a3 7
#define t0 8
#define t1 9
#define t2 10
#define t3 11
#define t4 12
#define t5 13
#define t6 14
#define t7 15
#define s0 16
#define s1 17
#define s2 18
#define s3 19
#define s4 20
#define s5 21
#define s6 22
#define s7 23
#define t8 24
#define t9 25
#define k0 26
#define k1 27
#define gp 28
#define sp 29
#define s8 30
#define fp 30
#define ra 31

#define LEAF_WEAK(a) \
	.weak a; \
	.type a, %function; \
	a:;

#define LEAF(a) \
	.global a; \
	.type a, %function; \
	a:;

#define END(a) \
	;

#define ENTRY(a) \
	.type a, %function; \
	a:;

	.set $C0_INDEX, $0
	.set $C0_RANDOM, $1
	.set $C0_ENTRYL0, $2
	.set $C0_ENTRYL1, $3
	.set $C0_CONTEXT, $4
	.set $C0_PAGEMASK, $5
	.set $C0_WIRED, $6
	.set $C0_BADVADDR, $8
	.set $C0_COUNT, $9
	.set $C0_ENTRYHI, $10
	.set $C0_COMPARE, $11
	.set $C0_STATUS, $12
	.set EN_IRQ_ONE, 0x1
	.set EN_IRQ_ZERO, 0xFFFFFFFE
	.set ERRORLVL_MASK, 0x6
	.set USER_MODE_MASK, 0x10
	.set NMI_IRQ, 0x80000
	.set IP_MASK, 0xFC00
	.set IP_MASK_OFFSET, 0xA
	.set EXC_MASK, 0x7C
	.set EXC_MASK_OFFSET, 0x2
	.set EXC_SYS, 0x8
	.set EN_FPU_CP1, 0x20000000
	.set $C0_CAUSE, $13
	.set IV_ONE, 0x800000
	.set IV_ZERO, 0xFF7FFFFF
	.set $C0_EPC, $14
	.set $C0_PRID, $15
	.set $C0_CONFIG, $16
	.set CACHE_EN_MASK, 0xFFFFFFF8
	.set HIGHMEM_CACHE_EN_MASK, 0x8FFFFFFF
	.set $C0_LLADR, $17
	.set $C0_ERROREPC, $30

	.set FLUSH_I, 0x1000
	.set FLUSH_D, 0x4000
	.set TR_CRAM, 0x2
	.set TR_CRAM_MASK, 0xFFFFFFFD


.macro SAVE_REGS
	addi	$sp, -CONTEXT_WORDS*4       /* Allocate saved context */
	sw	$1, (CONTEXT_R1 * 4) ($sp)      /*Save general registers */
	sw	$2, (CONTEXT_R2 * 4) ($sp)
	sw	$3, (CONTEXT_R3 * 4) ($sp)
	sw	$4, (CONTEXT_R4 * 4) ($sp)
	sw	$5, (CONTEXT_R5 * 4) ($sp)
	sw	$6, (CONTEXT_R6 * 4) ($sp)
	sw	$7, (CONTEXT_R7 * 4) ($sp)
	sw	$8, (CONTEXT_R8 * 4) ($sp)
	sw	$9, (CONTEXT_R9 * 4) ($sp)
	sw	$10, (CONTEXT_R10 * 4) ($sp)
	sw	$11, (CONTEXT_R11 * 4) ($sp)
	sw	$12, (CONTEXT_R12 * 4) ($sp)
	sw	$13, (CONTEXT_R13 * 4) ($sp)
	sw	$14, (CONTEXT_R14 * 4) ($sp)
	sw	$15, (CONTEXT_R15 * 4) ($sp)
	sw	$16, (CONTEXT_R16 * 4) ($sp)
	sw	$17, (CONTEXT_R17 * 4) ($sp)
	sw	$18, (CONTEXT_R18 * 4) ($sp)
	sw	$19, (CONTEXT_R19 * 4) ($sp)
	sw	$20, (CONTEXT_R20 * 4) ($sp)
	sw	$21, (CONTEXT_R21 * 4) ($sp)
	sw	$22, (CONTEXT_R22 * 4) ($sp)
	sw	$23, (CONTEXT_R23 * 4) ($sp)
	sw	$24, (CONTEXT_R24 * 4) ($sp)
	sw	$25, (CONTEXT_R25 * 4) ($sp)
	# Skip $26 - K0
	# Skip $27 - K1
	sw	$28, (CONTEXT_GP * 4) ($sp)
	# Skip $29 - SP
	sw	$30, (CONTEXT_FP * 4) ($sp)
	sw	$31, (CONTEXT_RA * 4) ($sp)

	mfhi	$k0                         /* Save special registers */
	sw	$k0, (CONTEXT_HI * 4) ($sp)

	mflo	$k0
	sw	$k0, (CONTEXT_LO * 4) ($sp)

	mfc0	$k0, $C0_EPC
	sw	$k0, (CONTEXT_PC * 4) ($sp)

	mfc0	$k0, $C0_STATUS
	sw	$k0, (CONTEXT_STATUS * 4) ($sp)
.endm

.macro RESTORE_REGS
	lw	$k0, (CONTEXT_LO * 4) ($sp)     /* Load special registers */
	mtlo	$k0

	lw	$k0, (CONTEXT_HI * 4) ($sp)
	mthi	$k0

	lw	$k0, (CONTEXT_PC * 4) ($sp)
	mtc0	$k0, $C0_EPC

	lw	$k0, (CONTEXT_STATUS * 4) ($sp)
	mtc0	$k0, $C0_STATUS

	lw	$1, (CONTEXT_R1 * 4) ($sp)      /* Load general registers */
	lw	$2, (CONTEXT_R2 * 4) ($sp)
	lw	$3, (CONTEXT_R3 * 4) ($sp)
	lw	$4, (CONTEXT_R4 * 4) ($sp)
	lw	$5, (CONTEXT_R5 * 4) ($sp)
	lw	$6, (CONTEXT_R6 * 4) ($sp)
	lw	$7, (CONTEXT_R7 * 4) ($sp)
	lw	$8, (CONTEXT_R8 * 4) ($sp)
	lw	$9, (CONTEXT_R9 * 4) ($sp)
	lw	$10, (CONTEXT_R10 * 4) ($sp)
	lw	$11, (CONTEXT_R11 * 4) ($sp)
	lw	$12, (CONTEXT_R12 * 4) ($sp)
	lw	$13, (CONTEXT_R13 * 4) ($sp)
	lw	$14, (CONTEXT_R14 * 4) ($sp)
	lw	$15, (CONTEXT_R15 * 4) ($sp)
	lw	$16, (CONTEXT_R16 * 4) ($sp)
	lw	$17, (CONTEXT_R17 * 4) ($sp)
	lw	$18, (CONTEXT_R18 * 4) ($sp)
	lw	$19, (CONTEXT_R19 * 4) ($sp)
	lw	$20, (CONTEXT_R20 * 4) ($sp)
	lw	$21, (CONTEXT_R21 * 4) ($sp)
	lw	$22, (CONTEXT_R22 * 4) ($sp)
	lw	$23, (CONTEXT_R23 * 4) ($sp)
	lw	$24, (CONTEXT_R24 * 4) ($sp)
	lw	$25, (CONTEXT_R25 * 4) ($sp)
	# Skip $26 - K0
	# Skip $27 - K1
	lw	$28, (CONTEXT_GP * 4) ($sp)
	# Skip $29 - SP
	lw	$30, (CONTEXT_FP * 4) ($sp)
	lw	$31, (CONTEXT_RA * 4) ($sp)
	addi	$sp, CONTEXT_WORDS*4        /* Deallocate saved context */
.endm

#endif
