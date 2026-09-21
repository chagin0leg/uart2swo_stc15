; ring.inc — запись пары (канал+байт) в tq[].
;
; Регистры / память:
;   r1        = tail  — индекс записи в tq[]
;   r0        = указатель на ячейку tq[tail]
;   A         — временная
;   _rx_pend  — 1 = пара готова
;   _rx_res[0]= канал, _rx_res[1] = байт
;
;   if (rx_pend) {
;       /* → 21 */  tq[tail++] = rx_res[0];
;                   tq[tail++] = rx_res[1];
;                   tail &= 15;
;                   rx_pend = 0;
;   } else {
;       /* → 21 */  /* пусто, nop-подушка */
;   }
;
ring:
	jb	_rx_pend, ring_put		; +3  → 3  ; есть пара → писать

	; пусто → 21
	nop							; +1  → 4
	nop							; +1  → 5
	nop							; +1  → 6
	nop							; +1  → 7
	nop							; +1  → 8
	nop							; +1  → 9
	nop							; +1  → 10
	nop							; +1  → 11
	nop							; +1  → 12
	nop							; +1  → 13
	nop							; +1  → 14
	nop							; +1  → 15
	nop							; +1  → 16
	nop							; +1  → 17
	nop							; +1  → 18
	sjmp	ring_done			; +3  → 21

ring_put:
	; пара готова → 21
	mov	a, r1					; +1  → 4  ; A = tail
	add	a, #_tq					; +2  → 6  ; A = &tq[tail]
	mov	r0, a					; +1  → 7  ; R0 → tq[tail]
	mov	a, _rx_res				; +2  → 9  ; A = канал
	mov	@r0, a					; +1  → 10 ; tq[tail] = канал
	inc	r0						; +1  → 11 ; R0 → tq[tail+1]
	mov	a, _rx_res + 1			; +2  → 13 ; A = байт
	mov	@r0, a					; +1  → 14 ; tq[tail+1] = байт
	inc	r1						; +1  → 15 ; tail++
	inc	r1						; +1  → 16 ; tail++
	mov	a, r1					; +1  → 17 ; A = tail
	anl	a, #0x0F				; +2  → 19 ; tail &= 15
	mov	r1, a					; +1  → 20 ; сохранить tail
	clr	_rx_pend				; +1  → 21 ; пара забрана

ring_done:
