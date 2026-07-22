#include "audio.h"

#define ADSR_ATTACK  0
#define ADSR_DECAY   1
#define ADSR_SUSTAIN 2
#define ADSR_RELEASE 3
#define ADSR_OFF     4

static void setup_phase(SpuAdsr *a, int phase)
{
    int rate, rate_mask, base_step;
    int32_t step;

    a->phase = phase;

    switch (phase) {
    case ADSR_ATTACK:
        rate = a->attack_rate;
        rate_mask = 0x7F;
        a->decreasing = 0;
        a->exponential = (a->adsr1 >> 15) & 1;
        break;
    case ADSR_DECAY:
        rate = a->decay_rate;
        rate_mask = 0x1F << 2;
        a->decreasing = 1;
        a->exponential = 1;
        break;
    case ADSR_SUSTAIN:
        rate = a->sustain_rate;
        rate_mask = 0x7F;
        a->decreasing = (a->adsr2 >> 14) & 1;
        a->exponential = (a->adsr2 >> 15) & 1;
        break;
    default:
        rate = a->release_rate;
        rate_mask = 0x1F << 2;
        a->decreasing = 1;
        a->exponential = (a->adsr2 >> 5) & 1;
        break;
    }

    a->counter = 0;
    a->counter_inc = 0x8000;

    base_step = 7 - (rate & 3);
    step = a->decreasing ? ~base_step : base_step;

    if (rate < 44) {
        step <<= (11 - (rate >> 2));
    } else if (rate >= 48) {
        a->counter_inc >>= ((rate >> 2) - 11);
        if ((rate & rate_mask) != rate_mask) {
            if (a->counter_inc < 1)
                a->counter_inc = 1;
        }
    }

    a->step = step;
}

void spu_adsr_key_on(SpuAdsr *a, uint16_t adsr1, uint16_t adsr2)
{
    a->level = 0;
    a->adsr1 = adsr1;
    a->adsr2 = adsr2;

    a->attack_rate  = (int)(((adsr1 >> 10) & 0x1F) << 2) | ((adsr1 >> 8) & 0x03);
    a->decay_rate   = (int)((adsr1 >> 4) & 0x0F) << 2;
    a->sustain_rate = (int)(((adsr2 >> 8) & 0x1F) << 2) | ((adsr2 >> 6) & 0x03);
    a->release_rate = (int)(adsr2 & 0x1F) << 2;

    {
        int sl = (int)(((adsr1 & 0x0F) + 1) * 0x800);
        a->sustain_level = sl < 0x7FFF ? sl : 0x7FFF;
    }

    setup_phase(a, ADSR_ATTACK);
}

void spu_adsr_key_off(SpuAdsr *a)
{
    if (a->phase == ADSR_OFF || a->phase == ADSR_RELEASE)
        return;
    setup_phase(a, ADSR_RELEASE);
}

int spu_adsr_tick(SpuAdsr *a)
{
    int32_t this_step, this_inc;
    int rate;

    if (a->phase == ADSR_OFF)
        return 0;

    if (a->counter_inc <= 0)
        return a->level;

    this_step = a->step;
    this_inc  = a->counter_inc;

    switch (a->phase) {
    case ADSR_ATTACK:  rate = a->attack_rate;  break;
    case ADSR_DECAY:   rate = a->decay_rate;   break;
    case ADSR_SUSTAIN: rate = a->sustain_rate; break;
    default:           rate = a->release_rate;  break;
    }

    if (a->exponential) {
        if (a->decreasing) {
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

    a->counter += this_inc;
    if (!(a->counter & 0x8000))
        return a->level;
    a->counter = 0;

    a->level += this_step;
    if (a->level > 0x7FFF) a->level = 0x7FFF;
    if (a->level < 0) a->level = 0;

    switch (a->phase) {
    case ADSR_ATTACK:
        if (a->level >= 0x7FFF) setup_phase(a, ADSR_DECAY);
        break;
    case ADSR_DECAY:
        if (a->level <= a->sustain_level) setup_phase(a, ADSR_SUSTAIN);
        break;
    case ADSR_RELEASE:
        if (a->level <= 0) a->phase = ADSR_OFF;
        break;
    }

    return a->level;
}
