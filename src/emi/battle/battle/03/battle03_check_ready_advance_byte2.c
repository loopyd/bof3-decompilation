#include "internal.h"

/* @source 0x801E19DC
 * @behavior Calls battle03_local_ready_or_helper2 and increments byte 2 of scratchpad pointer slot 0x44.
 */
void battle03_check_ready_advance_byte2(void) {
    battle03_local_ready_or_helper2();
    SPAD_PTR_SLOT(u8, 0x44)[2]++;
}
