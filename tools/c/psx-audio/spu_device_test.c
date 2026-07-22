#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "spu_device.h"

static uint32_t voice_address(unsigned int voice, unsigned int offset)
{
    return PSX_SPU_REGISTER_BASE + voice * PSX_SPU_VOICE_STRIDE + offset;
}

static int write_register(PsxSpu *spu, uint32_t address, uint16_t value)
{
    return psx_spu_write16(spu, address, value, 0);
}

static int upload_loop(PsxSpu *spu)
{
    uint8_t blocks[32];
    unsigned int i;

    memset(blocks, 0x11, sizeof(blocks));
    blocks[0] = 0x0c;
    blocks[1] = 4;
    blocks[16] = 0x1c;
    blocks[17] = 3;
    for (i = 18; i < sizeof(blocks); i++)
        blocks[i] = 0;

    return write_register(spu, PSX_SPU_TRANSFER_ADDRESS, 0) ||
           psx_spu_dma_write(spu, blocks, sizeof(blocks), 0);
}

static int configure_voice(PsxSpu *spu, unsigned int voice)
{
    return write_register(spu, PSX_SPU_MAIN_VOLUME_LEFT, 0x3fff) ||
           write_register(spu, PSX_SPU_MAIN_VOLUME_RIGHT, 0x3fff) ||
           write_register(spu,
                          voice_address(voice, PSX_SPU_VOICE_VOLUME_LEFT),
                          0x3fff) ||
           write_register(spu,
                          voice_address(voice, PSX_SPU_VOICE_VOLUME_RIGHT),
                          0x3fff) ||
           write_register(spu, voice_address(voice, PSX_SPU_VOICE_PITCH),
                          0xffff) ||
           write_register(spu,
                          voice_address(voice, PSX_SPU_VOICE_START_ADDRESS),
                          0) ||
           write_register(spu, voice_address(voice, PSX_SPU_VOICE_ADSR1),
                          0x000f) ||
           write_register(spu, voice_address(voice, PSX_SPU_VOICE_ADSR2), 0);
}

static int test_voice_23_loop_and_key_off(void)
{
    int16_t output[256 * 2];
    uint16_t endx;
    PsxSpu *spu = psx_spu_create();
    size_t i;
    int heard = 0;
    int failed = 1;

    if (!spu)
        return 1;
    if (upload_loop(spu) || configure_voice(spu, 23) ||
        write_register(spu, PSX_SPU_KEY_ON_HIGH, 1u << 7) ||
        psx_spu_render(spu, output, 256))
        goto done;

    for (i = 0; i < 256 * 2; i++) {
        if (output[i] != 0)
            heard = 1;
    }
    if (!heard || psx_spu_read16(spu, PSX_SPU_ENDX_HIGH, &endx) ||
        (endx & (1u << 7)) == 0)
        goto done;

    if (write_register(spu, PSX_SPU_KEY_OFF_HIGH, 1u << 7) ||
        psx_spu_render(spu, output, 16))
        goto done;
    for (i = 8 * 2; i < 16 * 2; i++) {
        if (output[i] != 0)
            goto done;
    }

    failed = 0;
done:
    psx_spu_destroy(spu);
    return failed;
}

static int test_render_arguments_and_logging(void)
{
    int16_t output[2];
    PsxSpu *spu = psx_spu_create();
    int failed;

    if (!spu)
        return 1;
    failed = psx_spu_render(NULL, output, 1) != -1 ||
             psx_spu_render(spu, NULL, 1) != -1 ||
             psx_spu_render(spu, NULL, 0) != 0 ||
             write_register(spu, PSX_SPU_KEY_ON_LOW, 1) != 0 ||
             psx_spu_write_count(spu) != 1 ||
             psx_spu_writes(spu)[0].address != PSX_SPU_KEY_ON_LOW;
    psx_spu_destroy(spu);
    return failed;
}

static int test_pitch_cap(void)
{
    int16_t capped[64 * 2];
    int16_t oversized[64 * 2];
    PsxSpu *first = psx_spu_create();
    PsxSpu *second = psx_spu_create();
    int failed = 1;

    if (!first || !second)
        goto done;
    if (upload_loop(first) || upload_loop(second) ||
        configure_voice(first, 0) || configure_voice(second, 0) ||
        write_register(first, voice_address(0, PSX_SPU_VOICE_PITCH), 0x4000) ||
        write_register(first, PSX_SPU_KEY_ON_LOW, 1) ||
        write_register(second, PSX_SPU_KEY_ON_LOW, 1) ||
        psx_spu_render(first, capped, 64) ||
        psx_spu_render(second, oversized, 64))
        goto done;
    failed = memcmp(capped, oversized, sizeof(capped)) != 0;

done:
    psx_spu_destroy(first);
    psx_spu_destroy(second);
    return failed;
}

int main(void)
{
    if (test_voice_23_loop_and_key_off()) {
        fprintf(stderr, "voice 23 loop/key-off test failed\n");
        return 1;
    }
    if (test_render_arguments_and_logging()) {
        fprintf(stderr, "render argument/logging test failed\n");
        return 1;
    }
    if (test_pitch_cap()) {
        fprintf(stderr, "pitch cap test failed\n");
        return 1;
    }
    return 0;
}
