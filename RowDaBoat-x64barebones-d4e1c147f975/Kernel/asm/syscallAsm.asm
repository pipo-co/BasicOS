;syscallAsm.asm 
;funcion asignada como rutina de atencion de la int 80h, se encarga de llamar a
; syscallDispatcher, el cual llama a la sysCall pertinente.
SECTION .text

EXTERN syscallDispatcher
GLOBAL syscallHandler

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

syscallHandler:
    pushState
    call syscallDispatcher
    popState
    iretq