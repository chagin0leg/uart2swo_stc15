; swo.inc — один бит SWO на P35.
;
; Регистры:
;   r7 = left  — сколько бит ещё слать в текущем байте (0 = нужен новый)
;   r5 = lo    — текущий байт, уходит младшим битом вперёд
;   r6 = head  — индекс чтения в tq[]
;   r1 = tail  — индекс записи (из ring)
;   C          — бит, который выведем на P35
;   A          — временная переменная
;
;   if (left == 9) {
;       /* → 20 (0) */  next = 0; left = 8; head = (head + 1) & 0x0F;
;   } else if (left != 0) {
;       /* → 20 (3) */  next = lo & 1; lo >>= 1; left--;
;   } else if (head != tail) {
;       /* → 20 (1) */  lo = tq[head]; left = 9; next = 1;  /* старт-бит */
;   } else {
;       /* → 20 (3) */  next = 1;  /* тишина / idle */
;   }
;   P35 = next;
;
swo:
	mov	a, r7					; +1  → 1  ; A = left
	jz	need_byte				; +3  → 4  ; left==0 → брать новый байт
	cjne	r7, #9, do_data		; +3  → 7  ; left!=9 → обычный бит данных
	clr	c						; +1  → 8  ; next = 0
	mov	r7, #8					; +1  → 9  ; left = 8
	inc	r6						; +1  → 10 ; head++
	mov	a, r6					; +1  → 11 ; A = head
	anl	a, #0x0F				; +2  → 13 ; head &= 15
	mov	r6, a					; +1  → 14 ; head = A
	ljmp	emit				; +4  → 18 ; на вывод

do_data:
	mov	a, r5					; +1  → 8  ; A = lo
	rrc	a						; +1  → 9  ; C = младший бит
	mov	r5, a					; +1  → 10 ; сохранить lo
	dec	r7						; +1  → 11 ; left--
	nop							; +1  → 12 ;
	nop							; +1  → 13 ;
	nop							; +1  → 14 ;
	ljmp	emit				; +4  → 18 ;

need_byte:
	mov	a, r6					; +1  → 5  ; A = head
	xrl	a, r1					; +1  → 6  ; A = head XOR tail
	jnz	do_fetch				; +3  → 9  ; очередь не пуста → fetch
	setb	c					; +1  → 10 ; next = 1
	nop							; +1  → 11 ;
	nop							; +1  → 12 ;
	nop							; +1  → 13 ;
	nop							; +1  → 14 ;
	ljmp	emit				; +4  → 18 ;

do_fetch:
	mov	a, r6					; +1  → 10 ; A = head
	add	a, #_tq					; +2  → 12 ; A = &tq[head]
	mov	r0, a					; +1  → 13 ; R0 → ячейка в IRAM
	mov	a, @r0					; +1  → 14 ; A = tq[head]
	mov	r5, a					; +1  → 15 ; lo = байт
	mov	r7, #9					; +1  → 16 ; left = 9
	setb	c					; +1  → 17 ; next = 1
	nop							; +1  → 18 ;

emit:
	mov	P35, c					; +2  → 20 ; вывести бит на пин
	
