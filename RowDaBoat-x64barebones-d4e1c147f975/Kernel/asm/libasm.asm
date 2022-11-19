;libasm.asm
;libreria general de kernel, permite hacer movimientos de datos con el mapa de entrada/salida
; desde C, implementa el acceso al reloj (RTC) y funcion auxiliar para el manejo de excepciones

GLOBAL cpuVendor
GLOBAL getRtc
GLOBAL inb
GLOBAL outb
GLOBAL getSP
GLOBAL enter_critical_region
GLOBAL leave_critical_region
GLOBAL callTimerTick

section .text
	
cpuVendor:
	push rbp
	mov rbp, rsp

	push rbx

	mov rax, 0
	cpuid


	mov [rdi], ebx
	mov [rdi + 4], edx
	mov [rdi + 8], ecx

	mov byte [rdi+13], 0

	mov rax, rdi

	pop rbx

	mov rsp, rbp
	pop rbp
	ret
;

getRtc:
    mov rax, rdi
    out 70h, al
    in al, 71h
	ret
;

inb:
	mov rdx,rdi	;port
	in al,dx	;value
	ret
;

outb:
	mov rax, rsi ;value
	mov rdx, rdi ;port
	out dx, al
	ret
;

getSP:
	mov rax, rsp
	ret
;

enter_critical_region:
	push rax

.loop:
	mov al, 1
	xchg al, [rdi]
	cmp al, 0
	jne .loop

	pop rax
	ret
;

leave_critical_region:
	mov BYTE [rdi], 0
	ret
;

callTimerTick:
	int 20h
	ret
;