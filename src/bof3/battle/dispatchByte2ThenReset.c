#include "bof3/battle/battle03_internal.h"

/* @source 0x801E1990
 * @behavior Dispatches the handler selected by scratchpad slot 0x44 byte 2, then calls resetScratchWhenGlobalBit4.
 * @status exact
 * @match 100.00
 * @residual none; live audit is instruction- and byte-exact.
 */
void dispatchByte2ThenReset(void) {
    D_801EB258[SPAD_PTR_SLOT(u8, 0x44)[2]]();
    resetScratchWhenGlobalBit4();
}
