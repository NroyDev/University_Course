@the information that tells arm-none-eabi-as what arch. to assemble to 
	.cpu arm926ej-s
	.fpu softvfp
@note, we must have the main function for the simulator's linker script
	.text
	.align	2   @align 4 byte
	.global	main
@ ---------------------------------------------------------------------
main:
@prologue
	STMFD	sp!, {fp, lr}
	ADD		fp, sp, #4

@code body
	BL 		start_deasm
    .include "test.s"

start_deasm:
	STMFD	sp!, {r4-r8}
	MOV		r4, lr;				@ start
	MOV		r5, lr;				@ current read pos
	ADR		r6, start_deasm		@ end+1

	LDR		r0, =PRINT_HEADER
	BL		printf
	LOOP:
		@ Check is it end
		CMP		r5, r6
		BEQ		EXIT

		@ Print Prefix
		SUB		r1, r5, r4
		LDR		r0, =PRINTF_PREFIX
		BL		printf
		@ Load a instruction
		LDR		r7, [r5], #4


		LSL		r8, r7, #4
		LSR		r8, r8, #30
		@ Check is it a Data Process instruction
		CMP		r8, #0					@ 00
		BNE		ELSEIF_LDST
			@ Check is it a MUL series instruction?
			@ https://i.sstatic.net/65xZHjJB.jpg
			@ MUL
			LSL		r0, r7, #6
			LSR		r0, r0, #28
			CMP		r0, #0
			LSLEQ	r0, r7, #24
			LSREQ	r0,	r0,	#28
			CMPEQ	r0, #9	@1001
			BEQ		ELSE

			@ Long MUL
			LSL		r0, r7, #6
			LSR		r0, r0, #29
			CMP		r0, #1	@001
			LSLEQ	r0, r7, #24
			LSREQ	r0, r0, #28
			CMPEQ	r0, #9	@1001
			BEQ		ELSE

			@ SWP
			LSL		r0, r7, #6
			LSR		r0, r0, #29
			CMP		r0, #2	@010
			LSLEQ	r0, r7, #10
			LSREQ	r0, r0, #30
			CMPEQ	r0,	#0	@00
			LSLEQ	r0,	r7, #20
			LSREQ	r0, r0, #24
			CMPEQ	r0, #9	@0000 1001
			BEQ		ELSE

			@ BX
			LSL		r0, r7, #4
			LSR		r0, r0, #8
			LDR		r1, =0x12FFF1	@0001 0010 1111 1111 1111 0001
			CMP		r0, r1
			BEQ		ELSE

			@ it is a Data Process instruction
			@ Take opcode
			LSL		r0, r7, #7
			LSR		r0, r0, #28
			
			ADR		r1, PRINT_DATAPROC_TABLE
			ADD		r1, r1, r0, LSL #2
			LDR		r0, =PRINT_STRING
			BL		printf
			B		LOOP

		ELSEIF_LDST:
		@ Check is it a Load/Store instruction
		CMP		r8, #1					@ 01
		BNE		ELSEIF_SWI
			LSL		r0, r7, #11
			LSR		r0, r0, #31
			CMP		r0, #0
			LDREQ	r0, =PRINT_STR
			LDRNE	r0, =PRINT_LDR
			BL		printf
			B		LOOP

		ELSEIF_SWI:
		@ Check is it a SWI instruction
		LSL		r8, r7, #4
		LSR		r8, r8, #28
		CMP		r8, #15					@ 1111
		BLNE	ELSE
			LSL		r1, r7, #8
			LSR		r1, r1, #8
			LDR		r0, =PRINT_SWI
			BL		printf
			B		LOOP
		
		ELSE:	@ other instruction
			LDR		r0, =PRINT_DASH
			BL		printf
			B		LOOP
	EXIT:	

	MOV		r0, #0
	LDMFD	sp!, {r4-r8}
@epilogue
	SUB		sp, fp, #4
	LDMFD	sp!, {fp, lr}
	BX		lr

@ ---------------------------------------------------------------------
@data section
PRINT_DASH:		.asciz "--\n"

PRINT_DATAPROC_TABLE:
	.asciz "AND"		@ 0
	.asciz "EOR"		@ 1
	.asciz "SUB"		@ 2
	.asciz "RSB"		@ 3
	.asciz "ADD"		@ 4
	.asciz "ADC"		@ 5
	.asciz "SBC"		@ 6
	.asciz "RSC"		@ 7
	.asciz "TST"		@ 8
	.asciz "TEQ"		@ 9
	.asciz "CMP"		@ 10
	.asciz "CMN"		@ 11
	.asciz "ORR"		@ 12
	.asciz "MOV"		@ 13
	.asciz "BIC"		@ 14
	.asciz "MVN"		@ 15
PRINT_LDR:		.asciz "LDR\n"
PRINT_STR:		.asciz "STR\n"
PRINT_SWI:		.asciz "SWI\t#%d\n"

PRINT_HEADER:	.asciz "PC\t\Instruction\n"
PRINTF_PREFIX:	.asciz "%d\t"
PRINT_STRING:	.asciz "%s\n"
.end
    
    