GLOBAL cpuVendor
GLOBAL getRtc
GLOBAL inb
GLOBAL outb

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