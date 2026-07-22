#include <stdint.h>
#include <stdlib.h>

#include "psx_machine.h"

static void write32(uint8_t *p, uint32_t value)
{
    p[0] = (uint8_t)value;
    p[1] = (uint8_t)(value >> 8);
    p[2] = (uint8_t)(value >> 16);
    p[3] = (uint8_t)(value >> 24);
}

int main(void)
{
    Psf1Image image = { 0 };
    PsxSpu *spu;
    PsxMachine *machine;
    const PsxSpuWrite *writes;
    int failed;

    image.ram = (uint8_t *)calloc(1, PSF1_RAM_SIZE);
    if (!image.ram)
        return 1;
    image.initial_pc = 0x80010000u;
    image.initial_sp = 0x801ffff0u;
    write32(image.ram + 0x10000, 0x3c081f80u); /* lui t0,0x1f80 */
    write32(image.ram + 0x10004, 0x35081c00u); /* ori t0,t0,0x1c00 */
    write32(image.ram + 0x10008, 0x24091234u); /* addiu t1,zero,0x1234 */
    write32(image.ram + 0x1000c, 0xa5090000u); /* sh t1,0(t0) */

    spu = psx_spu_create();
    machine = psx_machine_create(&image, spu);
    free(image.ram);
    if (!spu || !machine) {
        psx_machine_destroy(machine);
        psx_spu_destroy(spu);
        return 1;
    }
    failed = psx_machine_run(machine, 4) != PSX_MACHINE_OK;
    writes = psx_spu_writes(spu);
    failed = failed || psx_spu_write_count(spu) != 1 ||
             writes[0].cycle != 3 ||
             writes[0].address != PSX_SPU_REGISTER_BASE ||
             writes[0].value != 0x1234 ||
             psx_machine_pc(machine) != 0x80010010u;
    psx_machine_destroy(machine);
    psx_spu_destroy(spu);

    spu = psx_spu_create();
    if (!spu)
        return 1;
    failed = psx_spu_write16(spu, PSX_SPU_TRANSFER_ADDRESS, 2, 10) != 0 ||
             psx_spu_write16(spu, PSX_SPU_TRANSFER_FIFO, 0x3412, 11) != 0 ||
             psx_spu_write16(spu, PSX_SPU_TRANSFER_FIFO, 0x7856, 12) != 0 ||
             psx_spu_ram(spu)[16] != 0x12 ||
             psx_spu_ram(spu)[17] != 0x34 ||
             psx_spu_ram(spu)[18] != 0x56 ||
             psx_spu_ram(spu)[19] != 0x78;
    failed = failed ||
             psx_spu_write16(spu, PSX_SPU_CONTROL, 0x8000, 13) != 0 ||
             psx_spu_write16(spu, PSX_SPU_TRANSFER_CONTROL, 4, 14) != 0 ||
             psx_spu_ram(spu)[20] != 0;
    {
        const uint8_t dma[] = { 0xaa, 0xbb, 0xcc, 0xdd };
        failed = failed ||
                  psx_spu_write16(spu, PSX_SPU_TRANSFER_ADDRESS, 4, 15) != 0 ||
                  psx_spu_dma_write(spu, dma, sizeof(dma), 16) != 0 ||
                 psx_spu_ram(spu)[32] != 0xaa ||
                 psx_spu_ram(spu)[33] != 0xbb ||
                 psx_spu_ram(spu)[34] != 0xcc ||
                 psx_spu_ram(spu)[35] != 0xdd;
    }
    psx_spu_destroy(spu);
    return failed;
}
