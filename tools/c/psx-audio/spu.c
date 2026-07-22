#include "audio.h"

#define ADSR_ATTACK  0
#define ADSR_DECAY   1
#define ADSR_SUSTAIN 2
#define ADSR_RELEASE 3
#define ADSR_OFF     4

static const uint16_t pitch_table[193] = {
    4096, 4110, 4125, 4140, 4155, 4170, 4185, 4200,
    4216, 4231, 4246, 4261, 4277, 4292, 4308, 4323,
    4339, 4355, 4371, 4386, 4402, 4418, 4434, 4450,
    4466, 4482, 4499, 4515, 4531, 4548, 4564, 4581,
    4597, 4614, 4630, 4647, 4664, 4681, 4698, 4715,
    4732, 4749, 4766, 4783, 4801, 4818, 4835, 4853,
    4870, 4888, 4906, 4924, 4941, 4959, 4977, 4995,
    5013, 5031, 5050, 5068, 5086, 5105, 5123, 5142,
    5160, 5179, 5198, 5216, 5235, 5254, 5273, 5292,
    5311, 5331, 5350, 5369, 5389, 5408, 5428, 5447,
    5467, 5487, 5507, 5527, 5547, 5567, 5587, 5607,
    5627, 5648, 5668, 5688, 5709, 5730, 5750, 5771,
    5792, 5813, 5834, 5855, 5876, 5898, 5919, 5940,
    5962, 5983, 6005, 6027, 6049, 6070, 6092, 6114,
    6137, 6159, 6181, 6203, 6226, 6248, 6271, 6294,
    6316, 6339, 6362, 6385, 6408, 6431, 6455, 6478,
    6501, 6525, 6549, 6572, 6596, 6620, 6644, 6668,
    6692, 6716, 6741, 6765, 6789, 6814, 6839, 6863,
    6888, 6913, 6938, 6963, 6988, 7014, 7039, 7064,
    7090, 7116, 7141, 7167, 7193, 7219, 7245, 7271,
    7298, 7324, 7351, 7377, 7404, 7431, 7458, 7485,
    7512, 7539, 7566, 7593, 7621, 7648, 7676, 7704,
    7732, 7760, 7788, 7816, 7844, 7873, 7901, 7930,
    7958, 7987, 8016, 8045, 8074, 8103, 8133, 8162,
    8192
};

uint16_t spu_pitch_from_note(int note, int fine, int center, int shift)
{
    int fine_index = fine + shift;
    int semitone;
    int octave;
    uint32_t pitch;

    if (fine_index < 0)
        fine_index += 7;
    fine_index >>= 3;

    semitone = 0;
    if (fine_index > 15) {
        semitone = 1;
        fine_index -= 16;
    }

    semitone += note - (center - 60);
    pitch = pitch_table[16 * (semitone % 12) + fine_index];
    octave = semitone / 12 - 5;
    if (octave > 0)
        pitch <<= octave;
    else if (octave < 0)
        pitch >>= -octave;
    if (pitch > 0x4000)
        pitch = 0x4000;
    return (uint16_t)pitch;
}

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
