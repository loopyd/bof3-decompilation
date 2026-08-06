#include "internal.h"

/* @source 0x801E4D8C
 * @behavior Dispatches a handler selected by scratchpad work byte +0x02, then calls resetEnemyScratchWhenBit4.
 */
void dispatchByte2ThenReset2(void)
{
    D_801EB46C[SPAD_PTR_SLOT(u8, 0x44)[2]]();
    resetEnemyScratchWhenBit4();
}
