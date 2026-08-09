#include "bof3/battle/battle03_internal.h"

/* @source 0x801E4D8C
 * @behavior Dispatches a handler selected by scratchpad work byte +0x02, then calls resetEnemyScratchWhenBit4.
 * @status exact
 * @match 100.00
 * @residual none; live audit is instruction- and byte-exact.
 */
void dispatchByte2ThenReset2(void)
{
    D_801EB46C[SPAD_PTR_SLOT(u8, 0x44)[2]]();
    resetEnemyScratchWhenBit4();
}
