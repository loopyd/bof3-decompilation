#include "bof3/bof3.h"

/* Shared battle work record pointer in main RAM, outside this overlay's image.
 * This target's generated undefined_syms_auto.txt row D_801EB4E0 = 0x801EB4E0
 * binds the address, so the row below carries it and no target-map row is
 * needed. The body reads the record only at byte offset 0x09 through this byte
 * view. */
extern u8 *D_801EB4E0; /* @source 0x801EB4E0 @kind unknown */

/* Helper of the shared main executable at 0x801E5988, outside this overlay's
 * image and bound by this target's generated undefined_funcs_auto.txt row. This
 * payload never establishes its role, so the generated raw name stays. */
extern void func_801E5988(void); /* @source 0x801E5988 */

/* @source 0x801EF3A4
 * @behavior Work byte 2 handler of this overlay, the third entry of the
 *           thirteen-entry handler-pointer run based at 0x801F08A0: the word at
 *           0x801F08A8 selects this span, and the dispatcher at 0x801EF0C4
 *           scales the scratchpad work object's byte 2 by four and calls that
 *           word with no bounds test, so no jal word in this payload names this
 *           address. Reads the shared battle work record pointer at 0x801EB4E0
 *           once, decrements that record's byte 0x09 by one at byte width
 *           (lbu / addiu -1 / sb) and then invokes the shared helper at
 *           0x801E5988 unconditionally, the store filling the call delay slot.
 *           The pointer load precedes frame creation because the decrement needs
 *           no stack; the 0x18-byte frame only keeps $ra at 0x10 across the
 *           call. Takes no arguments, returns nothing and reads or writes no
 *           other state. The role of record byte 0x09 is not established in this
 *           payload, so the name describes the write. The 0x34-byte span carried
 *           the same words as the then-exact lift
 *           decrementGlobalWorkByteBAndInvokeHelperBmagicMagic05403_801EF624
 *           except in its two byte-offset immediates (0x09 here, 0x0B there, and
 *           those two words differed in that immediate alone), so its statement
 *           order, operand forms and declaration style were transcribed and the
 *           offset was written from these bytes. A scan of every extracted
 *           payload found no byte-identical copy of this span, so no clone was
 *           available.
 * @status exact
 * @match 100.00
 * @residual none
 */
void stateCountdownGlobalCall(void) {
  D_801EB4E0[0x09]--;
  func_801E5988();
}
