#include "bof3/ui/game00_internal.h"

extern u32 func_8014D8D4(u8 arg0);

/*
 * @source 0x801B73F4
 * @behavior Entry 0 of the D_801CD2F4 work-state table: submits the
 * route-index-selected selection effect id to the shared main-executable helper
 * func_8014D8D4 - 0x38 when the scratchpad work-area route index
 * (work->route_index_08) reads 3 and 0x39 otherwise - then publishes 3 into the
 * active record selector byte at D_80146250 + 0x11C (the index of the
 * per-entity factor table D_80181BD4 read by the next table entry
 * func_801B744C) and advances the work-area handler index byte at 0x03 to 1, the
 * next entry of that table. The selection is written as two identical calls in
 * the if/else arms, which the compiler cross-jumps into the original's single
 * jal: 0x39 materialises in the branch delay slot and 0x38 on the taken arm.
 * @status exact
 * @match 100.00
 * @residual none
 */
void func_801B73F4(void)
{
    struct GameWorkArea* work;

    work = SPAD_PTR_SLOT(struct GameWorkArea, 0x44u);
    if (work->route_index_08 == 3) {
        func_8014D8D4(0x38u);
    } else {
        func_8014D8D4(0x39u);
    }
    D_80146250[0x11C] = 3;
    g_game_work->pad_03 = 1;
}
