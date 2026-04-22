;op Operation function
;1 Maximum max(intA,intB)
;2 greatest common divisor gcd(intA,intB)
;3 least common multiply lcm(intA,intB)
;4 exponent intAintB

section .data
    buffer:     resb    256

    intA:       DD      0
    intB:       DD      0
    intOP:      DD      0
    intResult:  DD      0
    AND_STR:    DB      " and "
    AND_STR_LEN equ $-AND_STR
    IS_STR:     DB      " is "
    IS_STR_LEN  equ $-IS_STR
    DOT_STR     DB      ".", 10
    DOT_STR_LEN  equ $-DOT_STR
    ; ----------------------------------------------- functions -----------------------------------------------
    MAX_STR:    DB      "Function 1: Maximum of "                    ; "Function 1: Maximum of 4 and 5 is 20."
    MAX_STR_LEN equ $-MAX_STR
    GCD_STR:    DB      "Function 2: greatest common divisor of "    ; "Function 2: greatest common divisor of 4 and 5 is 20."
    GCD_STR_LEN equ $-GCD_STR
    LCM_STR:    DB      "Function 3: least common multiply of "      ; "Function 3: least common multiply of 4 and 5 is 20."
    LCM_STR_LEN equ $-LCM_STR
    EXP_STR:    DB      "Function 4: exponent of "                   ; "Function 4: exponent of 4 and 5 is 20."
    EXP_STR_LEN equ $-EXP_STR


section .text
	global _start

_start:
    ;
    ; read input first
	;
    ; ssize_t read(int fd, void buf[.count], size_t count);
    mov eax, 3          ; sys_read 的編號是 3
    mov ebx, 0          ; file descriptor 0 (stdin)
    mov ecx, buffer     ; 存放輸入的緩衝區位址
    mov edx, 64         ; 讀取長度
    int 0x80            ; 觸發系統呼叫

    mov esi, buffer     ; esi (Source Index)
    call func_parse_int
    mov [intA], eax
    call func_parse_int
    mov [intB], eax
    call func_parse_int
    mov [intOP], eax
    
    ; ----------------------
    ; Maximum max(intA,intB)
    ; ----------------------
    cmp dword [intOP], 1          
    jne NOT_MAX
	mov eax,4               ; The system call for write (sys_write)
	mov ebx,1               ; File descriptor 1 - standard output
	mov ecx,MAX_STR         ; Put the offset of hello in ecx
	mov edx,MAX_STR_LEN     ; helloLen is a constant, so we don't need to say
    int 80h                 ; Call the kernel
    ; end print
    mov eax, [intA]
    mov ebx, [intB]
    call func_max
    mov [intResult], eax
    jmp   EXIT
    NOT_MAX:

    ; --------------------------------------
    ; Greatest common divisor gcd(intA,intB)
    ; --------------------------------------
    cmp dword [intOP], 2
    jne NOT_GCD
	mov eax,4               ; The system call for write (sys_write)
	mov ebx,1               ; File descriptor 1 - standard output
	mov ecx,GCD_STR         ; Put the offset of hello in ecx
	mov edx,GCD_STR_LEN     ; helloLen is a constant, so we don't need to say
    int 80h                 ; Call the kernel
    ; end print
    mov eax, [intA]
    mov ebx, [intB]
    call func_gcd
    mov [intResult], eax
    jmp   EXIT
    NOT_GCD:

    ; ------------------------------------
    ; Least common multiply lcm(intA,intB)
    ; ------------------------------------
    cmp dword [intOP], 3
    jne NOT_LCM
	mov eax,4               ; The system call for write (sys_write)
	mov ebx,1               ; File descriptor 1 - standard output
	mov ecx,LCM_STR         ; Put the offset of hello in ecx
	mov edx,LCM_STR_LEN     ; helloLen is a constant, so we don't need to say
    int 80h                 ; Call the kernel
    ; end print
    mov eax, [intA]
    mov ebx, [intB]
    call func_lcm
    mov [intResult], eax
    jmp   EXIT
    NOT_LCM:

    ; ----------------------
    ; exponent (intA)^(intB)
    ; ----------------------
    cmp dword [intOP], 4
    jne NOT_EXP
	mov eax,4               ; The system call for write (sys_write)
	mov ebx,1               ; File descriptor 1 - standard output
	mov ecx,EXP_STR         ; Put the offset of hello in ecx
	mov edx,EXP_STR_LEN     ; helloLen is a constant, so we don't need to say
    int 80h                 ; Call the kernel
    ; end print
    mov eax, [intA]
    mov ebx, [intB]
    call func_exp
    mov [intResult], eax
    jmp   EXIT
    NOT_EXP:

    EXIT:
    mov eax, [intA]
    call func_print_int
	mov eax,4               ; The system call for write (sys_write)
	mov ebx,1               ; File descriptor 1 - standard output
	mov ecx,AND_STR         ; Put the offset of hello in ecx
	mov edx,AND_STR_LEN     ; helloLen is a constant, so we don't need to say
    int 80h                 ; Call the kernel
    mov eax, [intB]
    call func_print_int
	mov eax,4               ; The system call for write (sys_write)
	mov ebx,1               ; File descriptor 1 - standard output
	mov ecx,IS_STR         ; Put the offset of hello in ecx
	mov edx,IS_STR_LEN     ; helloLen is a constant, so we don't need to say
    int 80h                 ; Call the kernel
    mov eax, [intResult]
    call func_print_int
	mov eax,4               ; The system call for write (sys_write)
	mov ebx,1               ; File descriptor 1 - standard output
	mov ecx,DOT_STR         ; Put the offset of hello in ecx
	mov edx,DOT_STR_LEN     ; helloLen is a constant, so we don't need to say
    int 80h                 ; Call the kernel


	mov eax,1            ; The system call for exit (sys_exit)
	mov ebx,0            ; Exit with return "code" of 0 (no error)
	int 80h;


; ------------------
; input:    eax, ebx
; return:   eax
; ------------------
func_max:
    ; Prologue
    ; code body
    cmp eax, ebx
    jae MAX_INTA_LARGER_EQUAL
    mov eax, ebx
    MAX_INTA_LARGER_EQUAL:
    ; Epilogue
    ret

; ------------------
; input:    eax, ebx
; return:   eax
; use edx
; ------------------
func_gcd:
    cmp eax, ebx
    jae gcd_eax_larger
    mov edx, eax
    mov eax, ebx
    mov ebx, edx
    gcd_eax_larger:

    gcd_loop:
        cmp ebx, 0
        je  gcd_exit
        
        xor edx, edx
        div ebx             ; eax=eax/ebx, edx=eax%ebx
        mov eax, ebx        ; eax = ebx
        mov ebx, edx        ; ebx = (old eax)%ebx

        jmp gcd_loop

    gcd_exit:
    ret                     ; return eax


; ------------------
; input:    eax, ebx
; return:   eax
; use ecx edx esi edi
; ------------------
func_lcm:
    mov esi, eax
    mov edi, ebx
    call func_gcd   ; should avoid edx
    mov ecx, eax    ; ecd = gcd(eax, ebx)

    mov eax, esi
    xor edx, edx
    div ecx         ; eax = (old eax)/gcd((old eax), (old ebx))
    mul edi

    ret


; ------------------
; input:    eax, ebx
; return:   eax
; use ecx
; ------------------
func_exp:
    mov ecx, eax
    sub ebx, 1
    func_exp_loop:
    cmp ebx, 0
    jbe func_exp_exit
    mul ecx
    sub ebx, 1
    jmp func_exp_loop
    func_exp_exit:
    ret


; ------------------
; input:
; return:   eax
; uses ebx
; ------------------
func_parse_int:
    xor eax, eax
    xor ebx, ebx

    .parse_int_skip_spaces:
    mov bl, [esi]
    cmp bl, ' ' 
    je .parse_int_inc_ptr
    cmp bl, 10
    je .parse_int_inc_ptr
    
    jmp .parse_int_convert_loop

    .parse_int_inc_ptr:
    inc esi
    jmp .parse_int_skip_spaces

    .parse_int_convert_loop:
    ; check is it digit
    cmp bl, '0'
    jl .parse_int_done
    cmp bl, '9'
    jg .parse_int_done

    sub bl, '0'
    imul eax, 10
    add eax, ebx
    
    inc esi
    mov bl, [esi]
    jmp .parse_int_convert_loop

    .parse_int_done:
    ret



func_print_int:
    mov ecx, buffer
    add ecx, 255        ; end of buffer
    mov ebx, 10         ; 除數

    ; special case for 0
    cmp eax, 0
    jne .print_int_convert_loop
    mov byte [ecx], '0'
    dec ecx
    jmp .print_int_write_stdout

.print_int_convert_loop:
    xor edx, edx
    div ebx

    add dl, '0'
    mov [ecx], dl
    dec ecx

    cmp eax, 0
    jnz .print_int_convert_loop

.print_int_write_stdout:
    inc ecx

    mov edx, buffer
    add edx, 255
    sub edx, ecx
    inc edx

	mov eax,4               ; The system call for write (sys_write)
	mov ebx,1               ; File descriptor 1 - standard output
    int 80h                 ; Call the kernel

    ret