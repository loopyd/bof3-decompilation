#include "internal.h"

/* @source 0x800ADFBC */
// @behavior Dispatches an action-table handler when scratchpad state 5 is nonzero.
void func_800ADFBC(void)
{
    if (D_801462E8 & 0x800) {
        D_800B65FC[g_battle_work[5]].handler();
    }
}
