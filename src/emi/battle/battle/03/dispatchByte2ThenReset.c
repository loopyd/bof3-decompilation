#include "internal.h"

/* @source 0x801E1990
 * @behavior Dispatches the handler selected by scratchpad slot 0x44 byte 2, then calls resetScratchWhenGlobalBit4.
 */
void dispatchByte2ThenReset(void) {
    D_801EB258[SPAD_PTR_SLOT(u8, 0x44)[2]]();
    resetScratchWhenGlobalBit4();
}
