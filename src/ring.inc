; ring.inc — запись пары (канал + байт) в tq[] по флагу F0.
;
; Соглашение:
;   F0 = 1  — есть пара; поднимает uart, снимает ring
;   r1 = tail ∈ {0, 8, 10, 12, 14}  — чётный, 4-битный
;   tq[0..5] = 00 00 00 00 00 80 (заполняется при init)
;   _rx_res[0] = канал, _rx_res[1] = байт
;
;   r1 == 0  →  r1 := 6, писать в tq[6..7], r1 := 8 после записи
;   иначе    →  писать в tq[r1..r1+1], r1 := (r1 + 2) & 15
;   F0 := 0
;
;   _tq     — в idata ниже 0xF0
;   _rx_res — в direct-пространстве 0x00..0x7F
;   r1      — стартует чётным

ring: ; → 0 ; начало обработчика кольцевого буфера
	jnb	F0, ring_end		; +2  → 2  ; F0 == 0 → ring_end
	mov	a, r1				; +1  → 3  ; A = tail
	jnz	ring_nop			; +2  → 5  ; tail != 0 → ring_put
	mov	r1, #6				; +1  → 6  ; tail := 6 (write в tq[6..7])
	sjmp	ring_put		; +2  → 8

ring_nop: ; → 5 ; обычный путь когда не надо отправлять синхронизацию
	nop						; +1  → 6
	nop						; +1  → 7
	nop						; +1  → 8

ring_put: ; → 8 ; записать пару
	mov	a, r1				; +1  → 9  ; A = tail
	add	a, #_tq				; +1  → 10 ; A = &tq[tail]
	mov	r0, a				; +1  → 11 ; R0 = A
	mov	a, _rx_res			; +1  → 12 ; A = channel
	mov	@r0, a				; +1  → 13 ; tq[tail] = A
	inc	r0					; +1  → 14 ; R0 = &tq[tail] +1
	mov	a, _rx_res+1		; +1  → 15 ; A = byte
	mov	@r0, a				; +1  → 16 ; tq[tail+1] = A
	mov	a, r1				; +1  → 17 ; A = tail
	add	a, #2				; +1  → 18 ; A + 2
	anl	a, #0x0F			; +1  → 19 ; (A + 2) & 15
	mov	r1, a				; +1  → 20 ; tail = A
	clr	F0					; +1  → 21 ; F0 = 0
	nop						; +1  → 22
	nop						; +1  → 23
	nop						; +1  → 24
	nop						; +1  → 25
	nop						; +1  → 26
	nop						; +1  → 27
	nop						; +1  → 28
	nop						; +1  → 29

ring_end: ; → 2/29 ; выход из обработчика кольцевого буфера