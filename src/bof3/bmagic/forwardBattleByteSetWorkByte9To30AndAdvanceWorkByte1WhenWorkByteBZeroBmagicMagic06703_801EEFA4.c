#include "bof3/bof3.h"

/* Scratchpad work-object pointer cell 0x1F800044. This target's map binds the
 * address as g_battle_work, the name its other exact lifts use. The original
 * re-materialises the cell for every access, including the two accesses after
 * the call, because the callee may repoint the cell and because a byte store
 * through the loaded pointer may alias the cell itself. */
extern u8 *g_battle_work; /* @source 0x1F800044 @kind data */

/* Shared battle target byte in main RAM, outside this overlay's image. The
 * original reads it as an unsigned byte and passes it zero-extended, and this
 * target's generated undefined_syms_auto.txt already binds the address, so the
 * name needs no map row. */
extern u8 D_80146384; /* @source 0x80146384 @kind data */

/* Shared main-executable helper that forwards the battle target byte: its exact
 * callers pass that byte alone, and its role beyond receiving it is not
 * established here, so it keeps its address-qualified name. */
extern void func_800A9820(u8 value); /* @source 0x800A9820 @kind function */

/* @source 0x801EEFA4
 * @behavior Reads the scratchpad work-object pointer cell at 0x1F800044 once,
 *           before its frame is created, and returns immediately when the work
 *           object's byte at offset 0xB is non-zero (lbu, bnez with an empty
 *           delay slot). Otherwise it loads the shared battle target byte at
 *           0x80146384 unsigned and passes it zero-extended to the shared
 *           helper func_800A9820, re-reads the pointer cell and stores the
 *           countdown 0x1E into work byte 0x9 at byte width (addiu
 *           $v0,$zero,0x1E then sb), then re-reads the cell once more and
 *           increments work byte 0x1 by one at byte width (lbu, addiu 1, sb),
 *           advancing the work-byte-1 state that this overlay's dispatcher at
 *           0x801EEC24 indexes. Entry 2 (index 2) of the five-word code pointer
 *           run at 0x801EEC04 (0x801EEC94, 0x801EEF3C, 0x801EEFA4, 0x801EF010,
 *           0x801EF284), which that dispatcher copies into its own frame and
 *           calls with no arguments, so no jal word in the payload reaches this
 *           span: an overlay dispatch entry, not a called function. Takes no
 *           arguments, returns nothing, writes no other state, and the
 *           0x18-byte frame only keeps $ra at 0x10 across the call.
 *           Shape: statement order, operand forms and declaration style follow
 *           two exact lifts. The head comes from emi/bmagic/magic137/03 at
 *           0x801EEF64, which reads the same cell before its frame, guards on
 *           the same lbu/bnez test of work byte 0xB, passes the same unsigned
 *           battle byte to func_800A9820 and stores the same 0x1E countdown
 *           into byte 9. The reload-and-advance tail comes from this target's
 *           own exact lift at 0x801EEF3C. Neither sibling carries this body,
 *           so the guarded tail was written from this span's own bytes: no flag
 *           raise, no second call, and one advance of work byte 0x1 after the
 *           countdown store.
 *           A search for these 108 payload bytes across out/binaries returned
 *           only this target's own span, so no byte-identical twin exists and
 *           the clone lever was declined.
 * @status exact
 * @match 100.00
 * @residual none
 */
void forwardBattleByteSetWorkByte9To30AndAdvanceWorkByte1WhenWorkByteBZeroBmagicMagic06703_801EEFA4(
    void) {
  if (g_battle_work[0xB] == 0) {
    func_800A9820(D_80146384);
    g_battle_work[9] = 0x1E;
    g_battle_work[1]++;
  }
}
