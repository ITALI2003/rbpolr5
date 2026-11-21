/*
 * harness.c — обвязка для ESBMC: недетерминированный пакет от сети.
 * Пакет произвольной длины 0..64 байт с произвольным содержимым,
 * часть каналов может быть не зарегистрирована (слот = NULL).
 */
#include <stdint.h>
#include "vote_parser.h"

#define PKT_MAX 64

uint8_t  nondet_uchar(void);
uint16_t nondet_ushort(void);
_Bool    nondet_bool(void);

static vote_t slot_mem[N_CHANNELS];

int main(void)
{
    uint8_t  buf[PKT_MAX];
    uint16_t len = nondet_ushort();
    __ESBMC_assume(len <= PKT_MAX);
    for (int i = 0; i < PKT_MAX; i++)
        buf[i] = nondet_uchar();

    for (int k = 0; k < N_CHANNELS; k++)          /* канал мог не пройти регистрацию */
        g_slots[k] = nondet_bool() ? &slot_mem[k] : 0;

    vote_t v;
    if (parse_vote(buf, len, &v) == 0) {
        int16_t avg = vote_average(&v);
        (void)avg;
        store_vote(&v);
    }
    return 0;
}
