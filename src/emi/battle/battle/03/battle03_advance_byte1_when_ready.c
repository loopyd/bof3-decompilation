#include "internal.h"

/* @source 0x801E6BC8
 * @behavior increments scratch work state after a successful battle predicate.
 */
void battle03_advance_byte1_when_ready(void)
{
    u8** scratch_slots;

    if (func_8014D978() != 0u) {
        scratch_slots = SPAD_PTR_TABLE(u8);
        scratch_slots[0x11][1]++;
        scratch_slots = SPAD_PTR_TABLE(u8);
        scratch_slots[0x11][9] = 0x10;
    }
}
