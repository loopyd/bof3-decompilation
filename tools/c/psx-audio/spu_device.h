#ifndef PSX_AUDIO_SPU_DEVICE_H
#define PSX_AUDIO_SPU_DEVICE_H

#include <stddef.h>
#include <stdint.h>

#define PSX_SPU_REGISTER_BASE 0x1f801c00u
#define PSX_SPU_REGISTER_SIZE 0x200u
#define PSX_SPU_RAM_SIZE 0x80000u
#define PSX_SPU_TRANSFER_ADDRESS 0x1f801da6u
#define PSX_SPU_TRANSFER_FIFO 0x1f801da8u
#define PSX_SPU_CONTROL 0x1f801daau
#define PSX_SPU_TRANSFER_CONTROL 0x1f801dacu

#define PSX_SPU_VOICE_COUNT 24u
#define PSX_SPU_VOICE_STRIDE 0x10u
#define PSX_SPU_VOICE_VOLUME_LEFT 0x0u
#define PSX_SPU_VOICE_VOLUME_RIGHT 0x2u
#define PSX_SPU_VOICE_PITCH 0x4u
#define PSX_SPU_VOICE_START_ADDRESS 0x6u
#define PSX_SPU_VOICE_ADSR1 0x8u
#define PSX_SPU_VOICE_ADSR2 0xau
#define PSX_SPU_VOICE_ADSR_VOLUME 0xcu
#define PSX_SPU_VOICE_REPEAT_ADDRESS 0xeu

#define PSX_SPU_MAIN_VOLUME_LEFT 0x1f801d80u
#define PSX_SPU_MAIN_VOLUME_RIGHT 0x1f801d82u
#define PSX_SPU_KEY_ON_LOW 0x1f801d88u
#define PSX_SPU_KEY_ON_HIGH 0x1f801d8au
#define PSX_SPU_KEY_OFF_LOW 0x1f801d8cu
#define PSX_SPU_KEY_OFF_HIGH 0x1f801d8eu
#define PSX_SPU_ENDX_LOW 0x1f801d9cu
#define PSX_SPU_ENDX_HIGH 0x1f801d9eu

typedef struct {
    uint64_t cycle;
    uint32_t address;
    uint16_t value;
} PsxSpuWrite;

typedef struct PsxSpu PsxSpu;

PsxSpu *psx_spu_create(void);
void psx_spu_destroy(PsxSpu *spu);
void psx_spu_reset(PsxSpu *spu);
int psx_spu_read16(PsxSpu *spu, uint32_t address, uint16_t *value);
int psx_spu_write16(PsxSpu *spu, uint32_t address, uint16_t value,
                    uint64_t cycle);
int psx_spu_dma_write(PsxSpu *spu, const uint8_t *data, size_t size,
                      uint64_t cycle);
size_t psx_spu_write_count(const PsxSpu *spu);
const PsxSpuWrite *psx_spu_writes(const PsxSpu *spu);
const uint8_t *psx_spu_ram(const PsxSpu *spu);
/* Renders interleaved stereo at the native 44.1 kHz SPU rate. All MMIO writes
   remain readable and logged. Noise, pitch modulation, and reverb do not alter
   output yet; a sweep-mode volume contributes zero gain. */
int psx_spu_render(PsxSpu *spu, int16_t *stereo, size_t frames);
int psx_spu_voice_active(const PsxSpu *spu, unsigned int voice);

#endif
