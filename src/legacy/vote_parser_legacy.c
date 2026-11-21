/*
 * vote_parser_legacy.c — легаси-модуль арбитра TMR робота-курьера.
 * Принимает «голоса» каналов 1–3 (UDP/8888), разбирает пакет,
 * усредняет управляющие воздействия и сохраняет голос в слот канала.
 *
 * Формат пакета: [0] номер канала (1..3), [1] число осей n,
 *                далее n значений int16 big-endian (скорость левого/правого колеса, подъёмник...).
 *
 * ВНИМАНИЕ: исходная версия, содержит дефекты (найдены ESBMC):
 *   D1 CWE-190  переполнение uint8_t при расчёте длины пакета
 *   D2 CWE-787  запись за границу массива v[] при n > MAX_AXES
 *   D3 CWE-369  деление на ноль при n == 0
 *   D4 CWE-129  индекс слота из пакета без проверки (channel = 0 или > 3)
 *   D5 CWE-476  разыменование NULL, если канал не зарегистрирован
 */
#include <stdint.h>
#include <stddef.h>
#include "vote_parser.h"

vote_t *g_slots[N_CHANNELS];           /* заполняется при регистрации канала */

int parse_vote(const uint8_t *buf, uint16_t len, vote_t *out)
{
    out->channel = buf[0];
    out->n       = buf[1];
    uint8_t need = 2 + out->n * 2;     /* D1: при n >= 127 значение «заворачивается» */
    if (len < need)
        return -1;
    for (int i = 0; i < out->n; i++)   /* D2: n не сравнивается с MAX_AXES */
        out->v[i] = (int16_t)((buf[2 + 2 * i] << 8) | buf[3 + 2 * i]);
    return 0;
}

int16_t vote_average(const vote_t *v)
{
    int32_t s = 0;
    for (int i = 0; i < v->n; i++)
        s += v->v[i];
    return (int16_t)(s / v->n);        /* D3: n == 0 */
}

int store_vote(const vote_t *v)
{
    vote_t *slot = g_slots[v->channel - 1];   /* D4: channel не проверен */
    *slot = *v;                                /* D5: slot может быть NULL */
    return 0;
}
