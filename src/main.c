/**
 * main.c — инициализация.
 * Горячий цикл: swo, затем попеременно uart|ring.
 *
 * Кольцо tq[16]: только данные, wrap & 15 (синк пока убран).
 * RX: P32 @ 115200 → канал на n==2, байт на n==1.
 */

#include <stdint.h>
#include "stc15.h"

#define TQ 16

__data uint8_t tq[TQ];
__data uint8_t tail;

__data uint8_t rx_acc[5];
__data uint8_t rx_ph[5];   /* n */
__data uint8_t rx_bc[5];
__data uint8_t rx_due;
__data uint8_t rx_rr;
__data uint8_t rx_res[2];  /* [0]=канал, [1]=байт; pend=пара готова */
__bit rx_pend;


void loop(void);

void main(void)
{
    uint8_t i;

    P3M1 = 0x00;
    P3M0 = 0x20; /* P35 выход */
    P35 = 1;
    EA = 0;

    for (i = 0; i < 5; i++) {
        rx_bc[i] = 0;
        rx_ph[i] = 0;
        rx_acc[i] = 0;
    }
    rx_rr = 0;
    rx_due = 0xFF;
    rx_res[0] = 0;
    rx_res[1] = 0;
    rx_pend = 0;

    for (i = 0; i < TQ; i++)
        tq[i] = 0;

    /* тестовая пара до старта RX */
    tq[0] = 2; tq[1] = 'A';
    tq[2] = 2; tq[3] = 'B';
    tail = 4;

    loop();
}
