/*
 * vote_parser_mutant.c — МУТАНТ исправленной версии: из parse_vote() удалена проверка n > MAX_AXES
 * с формальной спецификацией на ACSL (проверка: Frama-C WP + RTE, ESBMC).
 */
#include <stdint.h>
#include <stddef.h>
#include "vote_parser.h"

vote_t *g_slots[N_CHANNELS];

/*@ predicate valid_vote(vote_t *v) =
      1 <= v->channel <= N_CHANNELS && 1 <= v->n <= MAX_AXES;
*/

/*@ requires len == 0 || \valid_read(buf + (0 .. len - 1));
    requires \valid(out);
    requires \separated(buf + (0 .. len - 1), out);
    assigns  *out;
    behavior ok:
      assumes buf != \null && len >= 2
           && 1 <= buf[0] <= N_CHANNELS && 1 <= buf[1] <= MAX_AXES
           && len >= 2 + 2 * buf[1];
      ensures \result == 0 && valid_vote(out);
      ensures out->channel == buf[0] && out->n == buf[1];
    behavior bad:
      assumes !(buf != \null && len >= 2
           && 1 <= buf[0] <= N_CHANNELS && 1 <= buf[1] <= MAX_AXES
           && len >= 2 + 2 * buf[1]);
      ensures \result == -1;
    complete behaviors; disjoint behaviors;
*/
int parse_vote(const uint8_t *buf, uint16_t len, vote_t *out)
{
    if (buf == NULL || len < 2)
        return -1;
    uint8_t ch = buf[0];
    uint8_t n  = buf[1];
    if (ch < 1 || ch > N_CHANNELS)                 /* D4 */
        return -1;
    if (n < 1)                                     /* МУТАЦИЯ: убрана проверка n > MAX_AXES */
        return -1;
    if ((uint32_t)len < 2u + 2u * (uint32_t)n)     /* D1: расчёт в 32 битах */
        return -1;
    out->channel = ch;
    out->n = n;
    /*@ loop invariant 0 <= i <= n;
        loop invariant out->channel == ch && out->n == n;
        loop assigns i, out->v[0 .. MAX_AXES - 1];
        loop variant n - i;
    */
    for (uint8_t i = 0; i < n; i++) {
        uint16_t raw = (uint16_t)(((uint16_t)buf[2 + 2 * i] << 8) | buf[3 + 2 * i]);
        out->v[i] = (int16_t)raw;
    }
    return 0;
}

/*@ requires \valid_read(v);
    requires valid_vote(v);
    assigns  \nothing;
    ensures  -32768 <= \result <= 32767;
*/
int16_t vote_average(const vote_t *v)
{
    int32_t s = 0;
    /*@ loop invariant 0 <= i <= v->n;
        loop invariant -32768 * i <= s <= 32767 * i;
        loop assigns i, s;
        loop variant v->n - i;
    */
    for (int i = 0; i < v->n; i++)
        s += v->v[i];
    return (int16_t)(s / v->n);                    /* n >= 1 по предусловию */
}

/*@ requires \valid_read(v) && valid_vote(v);
    requires \valid_read(g_slots + (0 .. N_CHANNELS - 1));
    requires \forall integer k; 0 <= k < N_CHANNELS && g_slots[k] != \null
             ==> \valid(g_slots[k]) && \separated(g_slots[k], v);
    assigns  *g_slots[v->channel - 1];
    ensures  \result == 0 || \result == -1;
    ensures  \result == -1 <==> \old(g_slots[v->channel - 1]) == \null;
*/
int store_vote(const vote_t *v)
{
    vote_t *slot = g_slots[v->channel - 1];        /* индекс 0..2 по предусловию */
    if (slot == NULL)                              /* D5 */
        return -1;
    *slot = *v;
    return 0;
}
