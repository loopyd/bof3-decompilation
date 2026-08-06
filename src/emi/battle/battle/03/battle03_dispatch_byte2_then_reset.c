#include "internal.h"

/* @source 0x801E1990
 * @behavior Dispatches the handler selected by scratchpad slot 0x44 byte 2, then calls battle03_reset_scratch_when_global_bit4.
 */
void battle03_dispatch_byte2_then_reset(void) {
    D_801EB258[SPAD_PTR_SLOT(u8, 0x44)[2]]();
    battle03_reset_scratch_when_global_bit4();
}
