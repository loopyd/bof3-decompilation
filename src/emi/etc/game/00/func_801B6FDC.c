#include "internal.h"

/*
 * @behavior Unless state byte D_80143BB0 is 2, clears bit 0x20 in the
 * record byte at D_80146250+0x118 and clears work-area byte 0x03.
 * @source 0x801B6FDC
 */
void func_801B6FDC(void)
{
    if (D_80143BB0 != 2) {
        D_80146250[0x118] &= 0xDF;
        g_game_work->pad_03 = 0;
    }
}
