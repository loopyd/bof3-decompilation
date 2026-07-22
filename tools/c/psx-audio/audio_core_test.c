#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "audio.h"

#define VAB_TONE_OFF 0x820
#define VAB_TONE_SIZE 32
#define VAB_TONES_PER_PROG 16

static void put_u16le(uint8_t *p, uint16_t value)
{
    p[0] = (uint8_t)value;
    p[1] = (uint8_t)(value >> 8);
}

static void put_u32le(uint8_t *p, uint32_t value)
{
    p[0] = (uint8_t)value;
    p[1] = (uint8_t)(value >> 8);
    p[2] = (uint8_t)(value >> 16);
    p[3] = (uint8_t)(value >> 24);
}

static int test_vab_program_attributes(void)
{
    const size_t pointer_off = VAB_TONE_OFF + VAB_TONES_PER_PROG * VAB_TONE_SIZE;
    const size_t size = pointer_off + 512;
    uint8_t *data = calloc(size, 1);
    uint8_t *tone;
    uint8_t *program;
    VabHeader header;
    int result = 1;

    if (!data)
        return 1;

    put_u32le(data, 0x56414270u);
    put_u16le(data + 0x12, 1);
    put_u16le(data + 0x14, 1);
    put_u16le(data + 0x16, 1);
    data[0x18] = 120;
    data[0x19] = 61;

    program = data + 0x20 + 13 * 16;
    program[0] = 1;
    program[1] = 105;
    program[4] = 70;

    tone = data + VAB_TONE_OFF;
    tone[2] = 127;
    tone[3] = 64;
    tone[4] = 60;
    tone[6] = 0;
    tone[7] = 127;
    put_u16le(tone + 20, 13);
    put_u16le(tone + 22, 1);
    put_u16le(data + pointer_off + 2, 2);

    memset(&header, 0, sizeof(header));
    if (vab_parse_vh(data, size, &header) != 0)
        goto done;
    if (header.program_count != 1 || header.tone_count != 1 ||
        header.declared_tone_count != 1 || header.vag_count != 1 ||
        header.master_vol != 120 ||
        header.master_pan != 61)
        goto done;
    if (header.tones[0].prog != 13 || header.tones[0].program_vol != 105 ||
        header.tones[0].program_pan != 70)
        goto done;

    result = 0;

done:
    free(data);
    return result;
}

static int test_pitch_table(void)
{
    if (spu_pitch_from_note(60, 0, 60, 0) != 4096)
        return 1;
    if (spu_pitch_from_note(61, 0, 60, 0) != 4339)
        return 1;
    if (spu_pitch_from_note(60, 0, 60, 127) != 4323)
        return 1;
    if (spu_pitch_from_note(72, 0, 60, 0) != 8192)
        return 1;
    if (spu_pitch_from_note(96, 0, 60, 0) != 32768)
        return 1;
    return 0;
}

int main(void)
{
    if (test_vab_program_attributes() != 0) {
        fprintf(stderr, "VAB program attribute test failed\n");
        return 1;
    }
    if (test_pitch_table() != 0) {
        fprintf(stderr, "pitch table test failed\n");
        return 1;
    }
    return 0;
}
