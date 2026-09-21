; loop.asm — каждый оборот: swo → (uart | ring) → top.
; Цель: swo → 20, uart|ring → 28, сумма 48 (пока НЕ выровнено).
;
; === Как на C ===
;
;   for (;;) {
;       swo();                 /* 20 */
;       if (k++ & 1) uart();
;       else         ring();   /* → 28 */
;   }
;
	.module loop
	.optsdcc -mmcs51 --model-small

	.globl _loop
	.globl _tq
	.globl _tail
	.globl _rx_acc
	.globl _rx_ph
	.globl _rx_bc
	.globl _rx_rr
	.globl _rx_res
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
	mov	r5, #0 		; lo = 0
	mov	r6, #0 		; head = 0
	mov	r1, _tail 	; tail = _tail
	clr	F0 			; флаг для выбора обработчика = 0
	clr	_rx_pend 	; флаг для обработки кольцевого буфера = 0

top:
	.include "swo.inc" 	; 20  → 20 ; обработка SWO
	cpl	F0 				;  1  → 21 ; флаг для выбора обработчика
	jnb	F0, do_ring 	;  3  → 24 ; если флаг установлен, то обработка UART
	.include "uart.inc" ; 21  → 21 ; обработка UART
	ljmp	top 		;  3  → 27 ; переход к началу цикла
do_ring:
	.include "ring.inc" ; 21  → 21 ; обработка кольцевого буфера
	ljmp	top 		;  3  → 27 ; переход к началу цикла
