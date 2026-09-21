; uart.inc — P32 @ 115200 (канал 2).
; FSM; пара в _rx_res[], pend на n==1 (ring заберёт оба байта).
; n = _rx_ph[2], bytes[2] = _rx_acc[2].
;
; === Как на C ===
;
;   if (n) n--;
;   if ((n & 3) == 0 && n >= 4 && n <= 32)
;       bytes[2] = (P32 << 7) | (bytes[2] >> 1);
;   else if (n == 2)
;       rx_res[0] = 2;           /* канал, ещё не pend */
;   else if (n == 1) {
;       rx_res[1] = bytes[2];
;       rx_pend = 1;             /* пара готова */
;   } else if (n == 0 && P32 == 0) {
;       n = 38;
;       bytes[2] = 0;
;   }
;
F1 = 0xD1

	mov	c, P32
	mov	F1, c

	mov	a, _rx_ph + 2
	jz	uart_arm

	dec	a
	mov	_rx_ph + 2, a
	mov	r2, a

	anl	a, #3
	jnz	uart_chk
	mov	a, r2
	clr	c
	subb	a, #33
	jnc	uart_chk
	mov	a, r2
	clr	c
	subb	a, #4
	jc	uart_chk
	mov	c, F1
	mov	a, _rx_acc + 2
	rrc	a
	mov	_rx_acc + 2, a
	sjmp	uart_done

uart_chk:
	mov	a, r2
	cjne	a, #2, uart_n1
	mov	_rx_res, #2
	sjmp	uart_done

uart_n1:
	cjne	a, #1, uart_n0
	mov	a, _rx_acc + 2
	mov	_rx_res + 1, a
	setb	_rx_pend
	sjmp	uart_done

uart_n0:
	mov	a, r2
	jnz	uart_done
uart_arm:
	mov	c, F1
	jc	uart_done
	mov	_rx_ph + 2, #38
	mov	_rx_acc + 2, #0

uart_done:
