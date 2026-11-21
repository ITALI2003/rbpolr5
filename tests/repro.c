/* repro.c — пакеты из контрпримеров ESBMC, запуск с ASan/UBSan.
 * usage: ./repro <номер теста 1..4> */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "vote_parser.h"

static vote_t slot_mem[N_CHANNELS];

int main(int argc, char **argv)
{
    uint8_t buf[64] = {0};
    uint16_t len = 0;
    int t = argc > 1 ? atoi(argv[1]) : 1;
    setvbuf(stdout, NULL, _IONBF, 0);   /* вывод до аварийного завершения */
    g_slots[0] = &slot_mem[0]; g_slots[1] = &slot_mem[1]; g_slots[2] = 0; /* канал 3 не зарегистрирован */
    switch (t) {
    case 1: buf[0] = 1; buf[1] = 155; len = 56; break; /* D1+D2: need = 312 mod 256 = 56 */
    case 2: buf[0] = 1; buf[1] = 0;   len = 4;  break; /* D3: n = 0 */
    case 3: buf[0] = 0; buf[1] = 2;   len = 6;  break; /* D4: channel = 0 */
    case 4: buf[0] = 3; buf[1] = 2;   len = 6;  break; /* D5: слот канала 3 = NULL */
    }
    printf("test %d: channel=%u n=%u len=%u\n", t, buf[0], buf[1], len);
    vote_t v;
    int r = parse_vote(buf, len, &v);
    printf("  parse_vote = %d\n", r);
    if (r == 0) {
        printf("  vote_average = %d\n", vote_average(&v));
        printf("  store_vote = %d\n", store_vote(&v));
    }
    return 0;
}
