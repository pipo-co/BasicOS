;sysCaller.asm
;funciones auxiliares de asm para realizar la int 80 y pasar los argumentos correctamente
SECTION .text

GLOBAL getTicksElapsed
GLOBAL drawPixel
GLOBAL sysWrite
GLOBAL getChar
GLOBAL setCursorPos
GLOBAL verticalPixelCount
GLOBAL horizontalPixelCount
GLOBAL getTime
GLOBAL sysBeep

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

%macro sysCaller 1
    pushState
	mov rcx, rdx
	mov rdx, rsi
	mov rsi, rdi
	mov rdi, %1
    ;rdi, rsi, rdx, rcx

    int 80h
    popState
	ret
%endmacro


getTicksElapsed:
	sysCaller 0

drawPixel:
	sysCaller 1

sysWrite:
	sysCaller 2

getChar:
	sysCaller 3

setCursorPos:
	sysCaller 4

verticalPixelCount:
	sysCaller 5

horizontalPixelCount:
	sysCaller 6

getTime:
	sysCaller 7

sysBeep:
	sysCaller 8