@the information that tells arm-none-eabi-as what arch. to assemble to 
	.cpu arm926ej-s
	.fpu softvfp

@this is code section
@note, we must have the main function for the simulator's linker script
	.text
	.align	2   @align 4 byte
	.global	main
main:
@prologue
    STMFD   sp!, {fp, lr}
    ADD     fp, sp, #4
@code body
    STMFD   sp!, {r4}           @ By APCS, register 4 are callee saved register. We need to save it before we wish to use it
    LDR     r4, [r1,#4]         @ Copy argv
    
    LDR     r0, =PRINTF_PREFIX
    BL      printf
    LOOP:
        @ To check is it reach end of string 
        LDRB    r1, [r4], #1
        CMP     r1, #0
        BEQ     EXIT

        @ To lower case
        ORR     r1, r1, #32
        @ To check it is a alphabetic char
        CMP     r1, #'a'
        BLT     LOOP
        CMP     r1, #'z'
        LDRLT   r0, =PRINTF_ARG0
        BLLT    printf
        B       LOOP
    EXIT:

    LDMFD   sp!, {r4}        @ restore callee saved register
@epilogue
    SUB     sp, fp, #4
    LDMFD   sp!, {fp, lr}
    BX      lr

@data section
PRINTF_PREFIX:  .asciz "prog1 result: "
PRINTF_ARG0:    .asciz "%c\0"