/* harness_eva.c — обвязка для Frama-C EVA: те же недетерминированные входы,
 * что и для ESBMC, но через встроенные функции Frama-C. */
#include <stdint.h>
#include "__fc_builtin.h"
#include "vote_parser.h"

#define PKT_MAX 64
static vote_t slot_mem[N_CHANNELS];

int main(void)
{
    uint8_t  buf[PKT_MAX];
    uint16_t len = (uint16_t)Frama_C_interval(0, PKT_MAX);
    for (int i = 0; i < PKT_MAX; i++)
        buf[i] = (uint8_t)Frama_C_interval(0, 255);
    for (int k = 0; k < N_CHANNELS; k++)
        g_slots[k] = Frama_C_nondet(0, 1) ? &slot_mem[k] : 0;

    vote_t v;
    if (parse_vote(buf, len, &v) == 0) {
        (void)vote_average(&v);
        store_vote(&v);
    }
    return 0;
}
