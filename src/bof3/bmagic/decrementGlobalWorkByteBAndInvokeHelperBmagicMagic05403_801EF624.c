#include "bof3/bof3.h"

void func_801E5988(void);

/* Shared battle work record pointer in main RAM, outside this overlay's image.
 * This target's generated undefined-syms map binds the address through the row
 * D_801EB4E0 = 0x801EB4E0, so the name needs no target-map row, and the record
 * is read only through the byte view below. */
extern u8 *D_801EB4E0; /* @source 0x801EB4E0 @kind unknown */

/* @source 0x801EF624
 * @behavior Work-byte-2 handler, entry 5 (the last) of this overlay's
 *           six-entry handler-pointer run at 0x801EEC14 (0x801EF238,
 *           0x801EF3A4, 0x801EF43C, 0x801EF4A8, 0x801EF534, 0x801EF624),
 *           which the overlay dispatcher at 0x801EF19C copies into its own
 *           frame and selects with byte 2 of the scratchpad work object at
 *           0x1F800044; the payload word at 0x801EEC28 is the only in-image
 *           reference to this address. Reads the shared battle work record
 *           pointer at 0x801EB4E0 once, decrements that record's byte 0xB by
 *           one at byte width (lbu 0xB / addiu -1 / sb 0xB) and then invokes
 *           the shared helper at 0x801E5988 unconditionally, the store filling
 *           the call's delay slot. The pointer load precedes frame creation
 *           because the decrement needs no stack; the 0x18-byte frame only
 *           keeps $ra across the call. Takes no arguments, returns nothing and
 *           reads or writes no other state. The 0x34 bytes here compared equal
 *           to the span at 0x801EF884 of bmagic/magic038, which was already an
 *           exact lift, so its statement order and operand forms were
 *           transcribed unchanged.
 * @status exact
 * @match 100.00
 * @residual none
 */
void decrementGlobalWorkByteBAndInvokeHelperBmagicMagic05403_801EF624(void) {
  D_801EB4E0[0xB]--;
  func_801E5988();
}
