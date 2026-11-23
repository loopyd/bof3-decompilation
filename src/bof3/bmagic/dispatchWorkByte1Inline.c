#include "bof3/bof3.h"

/* Scratchpad work-object pointer cell bound at 0x1F800044 by this target's map.
 * The original loads it once, ahead of the local handler table's stores, and
 * reads the object's unsigned byte 0x01 as the step selector, so the cell is
 * read once here too. */
extern u8 *g_battle_work; /* @source 0x1F800044 @kind data */

/* Entry 0 of the local pair is this overlay's own leading code body at
 * 0x801EEC90, still an `asm` row of this target's Splat, so it keeps its
 * address-anchored name. Entry 1 is the already-exact lift at 0x801EEDA4 of
 * this target. No jal and no aligned pointer word in this payload names
 * 0x801EEC38 (the 13-word code-pointer run at 0x801EEC04..0x801EEC34 never
 * holds it), so the overlay loader is the only caller of this body and both
 * handler roles stay structural. */
void func_801EEC90(void);
void forwardBattleRaiseFlag(
    void);

/* @source 0x801EEC38
 * @behavior Overlay step dispatcher: reads the scratchpad work-object pointer
 *           cell at 0x1F800044 once, seeds 0x801EEC90 into entry 0 and
 *           0x801EEDA4 into entry 1 of a two-entry local handler array in its
 *           own 0x20-byte frame, and invokes the entry selected by the work
 *           object's unsigned byte at offset 1, with no arguments. The index is
 *           unchecked, as in the original, which reads no other state and
 *           writes nothing outside its own frame; that frame only keeps $ra at
 *           0x18 across the indirect call. The work-cell load precedes the
 *           table's stores, and the 0x58-byte span ends at the jr at
 *           0x801EEC88, where this overlay's next declared body starts.
 * @status exact
 * @match 100.00
 * @residual none
 */
void dispatchWorkByte1Inline(void) {
  void (*handlers[2])(void) = {
      func_801EEC90,
      forwardBattleRaiseFlag};

  handlers[g_battle_work[1]]();
}
