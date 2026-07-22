#include <stdlib.h>
#include <string.h>

#include "spu_device.h"

struct PsxSpu {
    uint16_t registers[PSX_SPU_REGISTER_SIZE / 2];
    uint8_t ram[PSX_SPU_RAM_SIZE];
    uint32_t transfer_address;
    PsxSpuWrite *writes;
    size_t write_count;
    size_t write_capacity;
};

static int register_index(uint32_t address, size_t *index)
{
    uint32_t offset;
    if (address < PSX_SPU_REGISTER_BASE ||
        address >= PSX_SPU_REGISTER_BASE + PSX_SPU_REGISTER_SIZE ||
        (address & 1u) != 0)
        return -1;
    offset = address - PSX_SPU_REGISTER_BASE;
    *index = (size_t)(offset / 2);
    return 0;
}

PsxSpu *psx_spu_create(void)
{
    return (PsxSpu *)calloc(1, sizeof(PsxSpu));
}

void psx_spu_destroy(PsxSpu *spu)
{
    if (!spu)
        return;
    free(spu->writes);
    free(spu);
}

void psx_spu_reset(PsxSpu *spu)
{
    if (!spu)
        return;
    memset(spu->registers, 0, sizeof(spu->registers));
    memset(spu->ram, 0, sizeof(spu->ram));
    spu->transfer_address = 0;
    spu->write_count = 0;
}

int psx_spu_read16(PsxSpu *spu, uint32_t address, uint16_t *value)
{
    size_t index;
    if (!spu || !value || register_index(address, &index) != 0)
        return -1;
    *value = spu->registers[index];
    return 0;
}

int psx_spu_write16(PsxSpu *spu, uint32_t address, uint16_t value,
                    uint64_t cycle)
{
    size_t index;
    if (!spu || register_index(address, &index) != 0)
        return -1;
    if (spu->write_count == spu->write_capacity) {
        size_t capacity = spu->write_capacity ? spu->write_capacity * 2 : 256;
        PsxSpuWrite *writes = (PsxSpuWrite *)realloc(
            spu->writes, capacity * sizeof(*writes));
        if (!writes)
            return -1;
        spu->writes = writes;
        spu->write_capacity = capacity;
    }
    spu->registers[index] = value;
    if (address == PSX_SPU_TRANSFER_ADDRESS) {
        spu->transfer_address = ((uint32_t)value << 3) &
                                (PSX_SPU_RAM_SIZE - 1);
    } else if (address == PSX_SPU_TRANSFER_FIFO) {
        spu->ram[spu->transfer_address] = (uint8_t)value;
        spu->ram[(spu->transfer_address + 1) & (PSX_SPU_RAM_SIZE - 1)] =
            (uint8_t)(value >> 8);
        spu->transfer_address = (spu->transfer_address + 2) &
                                (PSX_SPU_RAM_SIZE - 1);
    }
    spu->writes[spu->write_count].cycle = cycle;
    spu->writes[spu->write_count].address = address;
    spu->writes[spu->write_count].value = value;
    spu->write_count++;
    return 0;
}

int psx_spu_dma_write(PsxSpu *spu, const uint8_t *data, size_t size,
                      uint64_t cycle)
{
    size_t i;
    (void)cycle;
    if (!spu || !data || (size & 1u) != 0)
        return -1;
    for (i = 0; i < size; i++) {
        spu->ram[spu->transfer_address] = data[i];
        spu->transfer_address = (spu->transfer_address + 1) &
                                (PSX_SPU_RAM_SIZE - 1);
    }
    return 0;
}

size_t psx_spu_write_count(const PsxSpu *spu)
{
    return spu ? spu->write_count : 0;
}

const PsxSpuWrite *psx_spu_writes(const PsxSpu *spu)
{
    return spu ? spu->writes : NULL;
}

const uint8_t *psx_spu_ram(const PsxSpu *spu)
{
    return spu ? spu->ram : NULL;
}
