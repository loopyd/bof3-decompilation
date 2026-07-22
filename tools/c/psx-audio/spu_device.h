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

#endif
