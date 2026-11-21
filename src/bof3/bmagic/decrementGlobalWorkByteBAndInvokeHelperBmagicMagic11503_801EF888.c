#include "bof3/bof3.h"

/* Shared battle work record pointer in main RAM, outside this overlay's image.
 * This target's generated undefined_syms_auto.txt binds the address through the
 * row D_801EB4E0 = 0x801EB4E0, so the name needs no target-map row, and the
 * record is read here only through the byte view below. */
extern u8 *D_801EB4E0; /* @source 0x801EB4E0 @kind unknown */

/* Shared no-argument helper of the main executable. The call site sets up no
 * argument register and this body discards the answer, since the epilogue
 * reloads only $ra, so the declared return type moves no instruction here; the
 * generated undefined_funcs_auto.txt already binds the address, so the
 * address-anchored prototype keeps the raw name and needs no target-map row. */
extern void func_801E5988(void);

/* @source 0x801EF888
 * @behavior Work-byte-2 handler, entry 7 and last of this overlay's eight-entry
 *           code pointer run based at 0x801EF8C4 (0x801EF130, 0x801EF27C,
 *           0x801EF49C, 0x801EF5C4, 0x801EF690, 0x801EF7A4, 0x801EF7F4,
 *           0x801EF888), reached through the state-byte-2 dispatcher
 *           dispatchStateByte2Tablef8c4BmagicMagic11503_801EF098, which scales
 *           byte 2 of the scratchpad work object at 0x1F800044 by four and
 *           calls the selected word with no arguments; the payload word at
 *           0x801EF8E0, the tail of the ten-word run that starts at 0x801EF8BC,
 *           is the only in-image reference to this address. Reads the shared
 *           battle work record pointer at 0x801EB4E0 once, decrements that
 *           record's byte 0xB by one at byte width (lbu 0xB / addiu -1 / sb 0xB)
 *           and then invokes the shared main-executable helper func_801E5988
 *           unconditionally, the store filling the call's delay slot. The
 *           pointer load precedes frame creation because the decrement needs no
 *           stack; the 0x18-byte frame only keeps $ra across the call. Takes no
 *           arguments, returns nothing and reads or writes no other state. The
 *           0x34 bytes here compared equal (cmp) to the span at 0x801EF5C4 of
 *           this same target, an exact lift, so its statement order, operand
 *           forms and declarations were transcribed unchanged; that body's span
 *           had in turn compared equal to 0x801EF624 of bmagic/magic054 and
 *           0x801EF884 of bmagic/magic038.
 * @status exact
 * @match 100.00
 * @residual none
 */
void decrementGlobalWorkByteBAndInvokeHelperBmagicMagic11503_801EF888(void) {
  D_801EB4E0[0xB]--;
  func_801E5988();
}
