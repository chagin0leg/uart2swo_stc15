; swo.inc — один бит SWO на P35.
;
; Регистры:
;   r7 = left  — сколько бит ещё слать в текущем байте (0 = нужен новый)
;   r5 = byte  — текущий байт, уходит младшим битом вперёд
;   r6 = head  — индекс чтения в tq[]
;   r1 = tail  — индекс записи (из ring)
;   C          — бит, который выведем на P35 (PSW)
;   A          — временная переменная для вычислений (ALU)
;
;   if (left != 0) { /* swo_tick */
;       left--;
;       if (left != 8) { /* swo_data  */
;           /* → 11 (1 nop) */  next = byte & 1; byte >>= 1;
;       } else { /* swo_start */
;           /* → 11 (0 nop) */  next = 0; head = (head + 1) & 15;
;       }
;   } else { /* swo_wait */
;       if (head != tail) { /* swo_next */
;           /* → 11 (0 nop) */  byte = tq[head]; left = 9; next = 1;
;       } else { /* swo_idle */
;			/* → 11 (3 nop) */ next = 1;
;   }
;   P35 = next;  /* +2  → 13 ; вывести бит на пин */
;

swo: ; → 0 ; вход в обработчик SWO
	cjne	r7, #0, swo_tick	; +2  → 2  ; if (left != 0) → swo_tick
; swo_wait: ; → 2 ; ожидание нового байта
	mov		a, r6				; +1  → 3  ; A = head
	cjne	a, 0x01, swo_next	; +2  → 5  ; if (head != tail) → swo_next
; swo_idle: ; → 5 ; пропуск бита данных
	setb	c					; +1  → 6  ; next = 1
	nop							; +1  → 7
	nop							; +1  → 8
	nop							; +1  → 9
	sjmp	emit				; +2  → 11

swo_tick: ; → 2 ; обработка бита данных
	dec		r7					; +1  → 3  ; left--
	cjne	r7, #8, swo_data	; +2  → 5  ; if (left != 8) → swo_data
; swo_start: ; → 5 ; начало байта
	clr		c					; +1  → 6  ; next = 0
	inc		r6					; +1  → 7  ; head++
	anl		0x06, #0x0F			; +2  → 9  ; r6 &= 15
	sjmp	emit				; +2  → 11

swo_data: ; → 5
	mov		a, r5				; +1  → 6  ; A = byte
	rrc		a					; +1  → 7  ; C = младший бит A
	mov		r5, a				; +1  → 8  ; byte = A
	nop							; +1  → 9
	sjmp	emit				; +2  → 11

swo_next: ; → 5
	add		a, #_tq				; +1  → 6  ; A = tq + head (A уже = head)
	mov		r0, a				; +1  → 7  ; R0 = адрес ячейки
	mov		a, @r0				; +1  → 8  ; A = tq[head]
	mov		r5, a				; +1  → 9  ; byte = tq[head]
	mov		r7, #9				; +1  → 10 ; left = 9
	setb	c					; +1  → 11 ; next = 1

emit: ; → 11
	mov	P35, c					; +2  → 13 ; вывести бит на пин