#include "bof3/bof3.h"

/* Scratchpad work-object pointer cell 0x1F800044. This target's map binds the
 * address as g_battle_work, the name its other exact lifts use. The original
 * re-materialises the cell for the byte-1 access after the call, because the
 * callee may repoint it, so the body reads the cell twice. */
extern u8 *g_battle_work; /* @source 0x1F800044 @kind data */

/* Shared battle target byte in main RAM, outside this overlay's image. The
 * original reads it as an unsigned byte and passes it zero-extended, and this
 * target's generated undefined_syms_auto.txt already binds the address, so the
 * name needs no map row. */
extern u8 D_80146384; /* @source 0x80146384 @kind data */

/* Shared main-executable helper that raises battler status flags: its exact
 * callers pass the battle target byte with one flag word, and it raises that
 * flag on the battlers the byte selects. */
extern void func_800A9610(u8 target, s32 flags); /* @source 0x800A9610 @kind function */

/* @source 0x801EEF3C
 * @behavior Counts down work byte 0x9 of the scratchpad work object reached
 *           through the pointer cell at 0x1F800044 with a byte-width
 *           read-modify-write (lbu, addiu -1, sb) and tests the truncated stored
 *           value (andi 0xFF, bnez with an empty delay slot). The cell load is
 *           hoisted above the frame setup because the countdown needs no stack.
 *           Only when that countdown reaches zero does it pass the shared battle
 *           target byte at 0x80146384 (unsigned load, zero-extended) and the
 *           flag word 0x20 to func_800A9610, which raises that flag on the
 *           battlers the byte selects, and then reload the work pointer cell and
 *           increment work byte 0x1 by one at byte width, advancing the
 *           work-byte-1 state that this overlay's dispatcher at 0x801EEC24
 *           indexes. That dispatcher copies the five-word code pointer run at
 *           0x801EEC04 (0x801EEC94, 0x801EEF3C, 0x801EEFA4, 0x801EF010,
 *           0x801EF284) into its own frame and calls the selected entry with no
 *           arguments, so this span is entry 1 of that run and no jal immediate
 *           in the payload reaches it: an overlay dispatch entry, not a called
 *           function. Takes no arguments, returns nothing, reads or writes no
 *           other state, and the 0x18-byte frame only keeps $ra at 0x10 across
 *           the call. Statement order, operand forms and declaration style
 *           follow the exact lift
 *           decrementWorkByte9RaiseBattleTargetFlag20AndAdvanceWorkByte1OnZero
 *           BmagicMagic13703_801EEEFC, whose 104-byte body equals this span in
 *           every word; literal 0x20, both byte offsets and the cell bindings
 *           were re-read from this span's own bytes, and a payload cmp proved
 *           the two 104-byte ranges identical.
 * @status exact
 * @match 100.00
 * @residual none
 */
void decrementWorkByte9RaiseBattleTargetFlag20AndAdvanceWorkByte1OnZeroBmagicMagic06703_801EEF3C(void) {
  if (--g_battle_work[9] == 0) {
    func_800A9610(D_80146384, 0x20);
    g_battle_work[1]++;
  }
}
