#include "bof3/battle/battle03_internal.h"

/* @source 0x801E6BC8
 * @behavior increments scratch work state after a successful battle predicate.
 * @status exact
 * @match 100.00
 * @residual none; live audit is instruction- and byte-exact.
 */
void advanceByte1WhenReady(void)
{
    u8** scratch_slots;

    if (func_8014D978() != 0u) {
        scratch_slots = SPAD_PTR_TABLE(u8);
        scratch_slots[0x11][1]++;
        scratch_slots = SPAD_PTR_TABLE(u8);
        scratch_slots[0x11][9] = 0x10;
    }
}
