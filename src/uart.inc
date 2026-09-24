; ===============================================================
;  Soft UART RX: 5 независимых каналов на одном 460800 Hz тике
;  Один сервис за тик.  F0 = «байт принят, main ещё не забрал».
;  Диспетчер: tick + next (half-range trick, см. ниже)
; ===============================================================

; ---------------------------------------------------------------
;  Скорости и производные константы
; ---------------------------------------------------------------
CH0_BAUD    = 115200
CH1_BAUD    =   9600
CH2_BAUD    =   9600
CH3_BAUD    =   9600
CH4_BAUD    =   9600

SAMPLE_INT_0 = 460800/CH0_BAUD      ;   4
SAMPLE_INT_1 = 460800/CH1_BAUD      ;  48
SAMPLE_INT_2 = 460800/CH2_BAUD      ;  48
SAMPLE_INT_3 = 460800/CH3_BAUD      ;  48
SAMPLE_INT_4 = 460800/CH4_BAUD      ;  48

; 1.5 бита от детекта старта до середины бита 0
START_DLY_0  = SAMPLE_INT_0 + SAMPLE_INT_0/2     ;   6
START_DLY_1  = SAMPLE_INT_1 + SAMPLE_INT_1/2     ;  72
START_DLY_2  = SAMPLE_INT_2 + SAMPLE_INT_2/2     ;  72
START_DLY_3  = SAMPLE_INT_3 + SAMPLE_INT_3/2     ;  72
START_DLY_4  = SAMPLE_INT_4 + SAMPLE_INT_4/2     ;  72

TOTAL_BAUD = CH0_BAUD+CH1_BAUD+CH2_BAUD+CH3_BAUD+CH4_BAUD
.ifgt TOTAL_BAUD - 172800
    .msg "TOTAL_BAUD exceeds 115200*1.5"
    .error 1
.endif

; ---------------------------------------------------------------
;  Переменные (замените адреса на свои)
; ---------------------------------------------------------------
_tick     = 0x40    ; 1 байт, монотонный счётчик обработанных тиков
_rx_next  = 0x41    ; 5 байт: абсолютное время следующего сервиса
_rx_bc    = 0x46    ; 5 байт: остаток бит (0 = канал свободен)
_rx_acc   = 0x4B    ; 5 байт: аккумулятор сдвига при приёме байта
_rx_res   = 0x50    ; 2 байта: [0]=номер канала, [1]=принятый байт

; ---------------------------------------------------------------
;  Макрос одного сервисного блока канала
; ---------------------------------------------------------------
.macro  SVC_BLOCK, NUM, PIN, CH, S_INT, S_DLY
svc'NUM:
        mov     a, _rx_bc+NUM
        jz      s'NUM'_arm              ; bc=0 → ищем старт-бит

        ; --- приём: сдвигаем очередной бит в аккумулятор ---
        mov     c, PIN
        mov     a, _rx_acc+NUM
        rrc     a                       ; LSB-first, как в UART
        mov     _rx_acc+NUM, a
        djnz    _rx_bc+NUM, s'NUM'_next ; ещё не все 8 бит

        ; --- байт принят целиком ---
        mov     _rx_res+0, #CH
        mov     _rx_res+1, a
        setb    F0                      ; отдаём main
        ajmp    uart_end

s'NUM'_next:
        mov     a, _tick
        add     a, #S_INT
        mov     _rx_next+NUM, a
        ajmp    uart_end

s'NUM'_arm:
        jb      PIN, s'NUM'_idle        ; линия молчит
        ; --- старт-бит пойман: 8 бит, ждём полтора бита ---
        mov     _rx_acc+NUM, #0
        mov     _rx_bc+NUM,  #8
        mov     a, _tick
        add     a, #S_DLY
        mov     _rx_next+NUM, a
        ajmp    uart_end

s'NUM'_idle:
        mov     a, _tick
        add     a, #S_INT
        mov     _rx_next+NUM, a
        ajmp    uart_end
.endm

; ---------------------------------------------------------------
;  Точка входа, вызывается на каждом тике 460800 Гц
; ---------------------------------------------------------------
uart: ; → 0 ; начало обработчика UART
        jnb     F0, u_go ; +2 → 2 ; F0 == 0 → u_go
        ajmp    uart_end ; +2 → 4 ;

u_go: ; → 2
        inc     _tick			; +1 → 3 ;

        mov     a, _tick
        clr     c
        subb    a, _rx_next+0
        jnb     acc.7, d0

        mov     a, _tick
        clr     c
        subb    a, _rx_next+1
        jnb     acc.7, d1

        mov     a, _tick
        clr     c
        subb    a, _rx_next+2
        jnb     acc.7, d2

        mov     a, _tick
        clr     c
        subb    a, _rx_next+3
        jnb     acc.7, d3

        mov     a, _tick
        clr     c
        subb    a, _rx_next+4
        jnb     acc.7, d4

        ajmp    uart_end

; ---- локальные трамплины: jnb ±127 → ajmp ±2048 ----
d0:     ajmp    svc0
d1:     ajmp    svc1
d2:     ajmp    svc2
d3:     ajmp    svc3
d4:     ajmp    svc4

; ---- тела сервисов (в обратном порядке — чтобы svc0 был ближе к трамплинам) ----
        SVC_BLOCK  4, P34, 5, SAMPLE_INT_4, START_DLY_4
        SVC_BLOCK  3, P33, 4, SAMPLE_INT_3, START_DLY_3
        SVC_BLOCK  2, P32, 3, SAMPLE_INT_2, START_DLY_2
        SVC_BLOCK  1, P31, 2, SAMPLE_INT_1, START_DLY_1
        SVC_BLOCK  0, P30, 1, SAMPLE_INT_0, START_DLY_0

uart_end:
        ret