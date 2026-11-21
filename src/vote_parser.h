#ifndef VOTE_PARSER_H
#define VOTE_PARSER_H
#include <stdint.h>

#define MAX_AXES   4      /* левое колесо, правое колесо, подъёмник, поворот */
#define N_CHANNELS 3      /* каналы TMR */

typedef struct {
    uint8_t channel;      /* 1..N_CHANNELS */
    uint8_t n;            /* 1..MAX_AXES   */
    int16_t v[MAX_AXES];
} vote_t;

extern vote_t *g_slots[N_CHANNELS];

int     parse_vote(const uint8_t *buf, uint16_t len, vote_t *out);
int16_t vote_average(const vote_t *v);
int     store_vote(const vote_t *v);

#endif
