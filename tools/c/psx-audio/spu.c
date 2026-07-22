#include "audio.h"

#define ADSR_ATTACK  0
#define ADSR_DECAY   1
#define ADSR_SUSTAIN 2
#define ADSR_RELEASE 3
#define ADSR_OFF     4

void spu_adsr_key_on(SpuAdsr *a, uint16_t adsr1, uint16_t adsr2)
{
    a->phase = ADSR_ATTACK;
    a->level = 0;
    a->adsr1 = adsr1;
    a->adsr2 = adsr2;

    a->attack_rate   = (((adsr1 >> 10) & 0x1F) << 2) | ((adsr1 >> 8) & 0x03);
    a->decay_rate    = ((adsr1 >> 4) & 0x0F) << 2;
    a->sustain_rate  = (((adsr2 >> 8) & 0x1F) << 2) | ((adsr2 >> 6) & 0x03);
    a->release_rate  = (adsr2 & 0x1F) << 2;
    a->sustain_level = ((adsr1 & 0x0F) + 1) << 11;
}

void spu_adsr_key_off(SpuAdsr *a)
{
    a->phase = ADSR_RELEASE;
}

int spu_adsr_tick(SpuAdsr *a)
{
    int rate, dec, exp_flag;

    if (a->phase == ADSR_OFF)
        return 0;

    switch (a->phase) {
    case ADSR_ATTACK:
        rate     = a->attack_rate;
        dec      = 0;
        exp_flag = (a->adsr1 >> 15) & 1;
        break;
    case ADSR_DECAY:
        rate     = a->decay_rate;
        dec      = 1;
        exp_flag = 1;
        break;
    case ADSR_SUSTAIN:
        rate     = a->sustain_rate;
        dec      = (a->adsr2 >> 14) & 1;
        exp_flag = (a->adsr2 >> 15) & 1;
        break;
    default:
        rate     = a->release_rate;
        dec      = 1;
        exp_flag = (a->adsr2 >> 5) & 1;
        break;
    }

    {
        int32_t counter_inc = 0x8000;
        int base_step = 7 - (rate & 3);
        int32_t step  = dec ? ~base_step : base_step;
        int32_t this_step, this_inc;

        if (rate < 44)
            step <<= 11 - (rate >> 2);
        else if (rate >= 48)
            counter_inc >>= (rate >> 2) - 11;

        this_step = step;
        this_inc  = counter_inc;

        if (exp_flag) {
            if (dec) {
                this_step = (this_step * a->level) >> 15;
            } else if (a->level >= 0x6000) {
                if (rate < 40)
                    this_step >>= 2;
                else if (rate >= 44)
                    this_inc >>= 2;
                else {
                    this_step >>= 1;
                    this_inc >>= 1;
                }
            }
        }

        a->level += this_step;
        if (a->level > 0x7FFF)
            a->level = 0x7FFF;
        if (a->level < 0)
            a->level = 0;

        (void)this_inc;
    }

    switch (a->phase) {
    case ADSR_ATTACK:
        if (a->level >= 0x7FFF)
            a->phase = ADSR_DECAY;
        break;
    case ADSR_DECAY:
        if (a->level <= a->sustain_level)
            a->phase = ADSR_SUSTAIN;
        break;
    case ADSR_RELEASE:
        if (a->level <= 0)
            a->phase = ADSR_OFF;
        break;
    }

    return a->level;
}
