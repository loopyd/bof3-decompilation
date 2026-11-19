#include "bof3/bof3.h"

/* Shared battle work record pointer in main RAM, outside this overlay's image.
 * The target's generated undefined-syms map already binds the address through
 * the row D_801EB4E0 = 0x801EB4E0, so the name needs no target-map row, and
 * the record is read only through the byte view below. */
extern u8 *D_801EB4E0; /* @source 0x801EB4E0 @kind unknown */

/* Helper of the main executable; the sibling bmagic lifts declare the same
 * prototype per caller, so no shared header owns it. */
void func_801E5988(void);

/* @source 0x801EF8A4
 * @behavior Work-byte-2 handler, entry 7 of this overlay's eight-entry
 *           handler-pointer run based at 0x801EF8E0, which the step wrapper at
 *           0x801EEEE8 selects with the scratchpad work object's unsigned byte
 *           2 scaled by four; the table word at 0x801EF8FC is the only
 *           in-image reference to this address. Reads the shared battle work
 *           record pointer at 0x801EB4E0 once, decrements that record's byte
 *           0xB by one at byte width (lbu 0xB, addiu -1, sb 0xB) and then
 *           invokes the shared helper at 0x801E5988 unconditionally, the store
 *           filling the call's delay slot. The pointer load precedes frame
 *           creation because the decrement needs no stack; the 0x18-byte frame
 *           only keeps $ra across the call. Takes no arguments, returns nothing
 *           and reads or writes no other state. This span's 52 bytes compared
 *           equal to the lifted span at 0x801EF3D8 of this overlay, so that
 *           function's statement order and operand forms were transcribed
 *           unchanged.
 * @status exact
 * @match 100.00
 * @residual none
 */
void decrementGlobalWorkByteBAndInvokeHelperBmagicMagic04500_801EF8A4(void) {
  D_801EB4E0[0xB]--;
  func_801E5988();
}
