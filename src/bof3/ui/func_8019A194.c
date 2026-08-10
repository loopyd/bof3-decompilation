#include "bof3/ui/game00_internal.h"

/*
 * @behavior Finds the current world state ID in the first byte of eleven
 * 0x1C-byte records and returns the matching index, or 11 when absent.
 * @source 0x8019A194
 * @status exact
 * @match 100.00
 * @residual none; live audit is instruction- and byte-exact.
 */
u8 func_8019A194(void)
{
    s32 index;
    s32 offset;
    s32 world_id;

    index = 0;
    world_id = D_80143F00;
    offset = 0;
    while (1) {
        if (world_id == *(u8 *)((u8 *)D_801C7F74 + offset)) {
            break;
        }
        index++;
        if (index >= 11) {
            break;
        }
        offset += 0x1C;
    }
    return index;
}
