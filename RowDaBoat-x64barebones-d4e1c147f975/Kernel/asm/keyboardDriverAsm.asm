;keyboard.asm
;Funciones auxiliares de interaccion con el mapa entrada/salida 
; utilizadas por el keyboard driver

GLOBAL keyboardActivated
GLOBAL getKeyboardScancode

section .text

;-----------------------------------------------------------
; keyboardActivated - Detecta si hay un codigo de teclado en el buffer
;-----------------------------------------------------------
; Return: 1 de haber codigo en el buffer, 0 sino.
;-----------------------------------------------------------
keyboardActivated:
	mov rax, 0
	in al, 64h
	and al, 01h
	ret

;-----------------------------------------------------------
; getKeyboardChar - Devuelve el codigo de teclado del buffer.
;-----------------------------------------------------------
; Return: Codigo del teclado
;-----------------------------------------------------------
getKeyboardScancode:
	mov rax, 0
	in al, 60h
	ret