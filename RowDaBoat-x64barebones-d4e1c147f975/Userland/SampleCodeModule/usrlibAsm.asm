;usrlibAsm.asm
GLOBAL getRegs
EXTERN printRegs

SECTION .text

%macro pushState 0
	push rbx
	push rcx
	push rdx
	push rbp
	push rdi
	push rsi
	push r8
	push r9
	push r10
	push r11
	push r12
	push r13
	push r14
	push r15
%endmacro

%macro popState 0
	pop r15
	pop r14
	pop r13
	pop r12
	pop r11
	pop r10
	pop r9
	pop r8
	pop rsi
	pop rdi
	pop rbp
	pop rdx
	pop rcx
	pop rbx
%endmacro

;funcion para obtener los valores de todos los registros y asi poder imprimirlos (inforeg)
getRegs:
    mov [regsSpace], rax
    mov [regsSpace + 1*8], rbx
    mov [regsSpace + 2*8], rcx
    mov [regsSpace + 3*8], rdx
    mov [regsSpace + 4*8], rbp
    mov [regsSpace + 5*8], rdi
    mov [regsSpace + 6*8], rsi
    mov [regsSpace + 7*8], r8
    mov [regsSpace + 8*8], r9
    mov [regsSpace + 9*8], r10
    mov [regsSpace + 10*8], r11
    mov [regsSpace + 11*8], r12
    mov [regsSpace + 12*8], r13
    mov [regsSpace + 13*8], r14
    mov [regsSpace + 14*8], r15
	mov rax, [rsp] ;supuestamente IP
    mov [regsSpace + 15*8], rax
    add rsp, 8
	mov [regsSpace + 16*8], rsp ;supuestamente el valor que tenia rsp
    sub rsp, 8

    mov rdi, regsSpace
	call printRegs

    ret


section .bss
regsSpace: resb 17*8