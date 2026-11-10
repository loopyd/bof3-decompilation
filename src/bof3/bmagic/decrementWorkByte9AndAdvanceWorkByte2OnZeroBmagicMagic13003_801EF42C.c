#include "bof3/bof3.h"

extern u8 *g_battle_work; /* @source 0x1F800044 @kind data */

/* @source 0x801EF42C
 * @behavior Decrements work byte 9 through the scratchpad pointer cell at
 * 0x1F800044. When the stored byte reaches zero, reloads the pointer cell
 * and increments work byte 2 to advance the handler step. Takes no arguments
 * and returns nothing. The byte store may alias the pointer cell, so the
 * second access reloads it. Payload cmp proved the 76-byte span identical
 * to emi/bmagic/magic096/03 at 0x801EF394; this body follows that sibling's
 * statement order and retains this target's g_battle_work binding.
 * @status exact
 * @match 100.00
 * @residual none
 */
void decrementWorkByte9AndAdvanceWorkByte2OnZeroBmagicMagic13003_801EF42C(void) {
    if (--g_battle_work[9] == 0) {
        g_battle_work[2]++;
    }
}
