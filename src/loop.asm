; loop.asm — главный цикл программы.
; Цель: swo → 13, uart + ring → 32, переход к началу цикла → 3, сумма 48.
;

	.module loop
	.optsdcc -mmcs51 --model-small

	.globl _loop
	.globl _tq
	.globl _tail
	; .globl _rx_acc
	.globl _rx_ph
	; .globl _rx_bc
	.globl _rx_due
	.globl _rx_rr
	; .globl _rx_res
	.globl _rx_pend

	.area CSEG (CODE)

P30 = 0xB0
P31 = 0xB1
P32 = 0xB2
P33 = 0xB3
P34 = 0xB4
P35 = 0xB5

_loop::
	mov	r7, #0 		; left = 0
	mov	r5, #0 		; byte = 0
	mov	r6, #0 		; head = 0
	mov	r1, _tail 	; tail = _tail
	clr	F0 			; флаг для выбора обработчика = 0
	clr	_rx_pend 	; флаг для обработки кольцевого буфера = 0

top: ; → 0 ; начало цикла
	.include "swo.inc" 	; +13 → 13 ; обработка SWO
	.include "uart.inc" ; ??  → ?? ; обработка UART
	.include "ring.inc" ; ??  → ?? ; обработка кольцевого буфера
	ljmp	top 		; +3  → 48 ; переход к началу цикла
