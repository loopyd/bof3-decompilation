#include "bof3/bof3.h"

/* Scratchpad work-object pointer cell shared by the battle and bmagic
 * overlays; this target's map binds it at 0x1F800044. The original
 * re-materializes the cell at both access groups (lui $v0,0x1F80 /
 * lw $v0,0x44($v0)) because the byte stores through the loaded pointers may
 * alias the cell, so each access stays on the named cell rather than on a
 * cached pointer local. */
extern u8 *g_battle_work; /* @source 0x1F800044 @kind data */

/* Shared battle work record pointer in main RAM, outside this overlay's
 * image. This target's generated undefined-syms map already binds the address
 * through the row D_801EB4E0 = 0x801EB4E0, so the name needs no target-map
 * row, and the record is read only through the byte view below. */
extern u8 *D_801EB4E0; /* @source 0x801EB4E0 @kind unknown */

/* @source 0x801EF2CC
 * @behavior Tests the scratchpad work object's unsigned byte at offset 0xB
 *           (lui $v0,0x1F80 / lw $v0,0x44($v0), load-delay nop / lbu
 *           $v0,0xB($v0) / bnez to the return, empty delay slot) and returns
 *           without writing anything when it is nonzero. When it is zero the
 *           body loads the shared battle work record pointer at 0x801EB4E0
 *           once (lui $v1,%hi / lw $v1,%lo, load-delay nop), decrements that
 *           record's byte at offset 0xB by one at byte width (lbu / addiu
 *           $v0,$v0,-1 / sb), then re-reads the scratchpad cell with its own
 *           lui/lw pair and increments the work object's byte at offset 0x2 at
 *           byte width (lbu / addiu $v0,$v0,1 / sb), advancing the dispatcher
 *           on work byte 2. Takes no arguments, returns nothing, keeps no
 *           frame and touches no other state; the 0x5C-byte range is a leaf
 *           ending in a plain jr $ra with an empty delay slot. The payload
 *           word at 0x801EEC20 holds this address: the fifth of the six-entry
 *           work-byte-2 handler run based at 0x801EEC10, which the dispatcher
 *           at 0x801EF020 copies into its own frame and indexes with the work
 *           object's byte 2, and the eighth word of the leading nine-word
 *           pointer run 0x801EEC04..0x801EEC27. No jal in this payload reaches
 *           it, which is the dispatch-table shape rather than a missing
 *           caller. No other overlay in this family holds these bytes: the
 *           0x5C-byte span occurs once across the bmagic payloads, so the seed
 *           is this target's exact sibling
 *           decrementWorkByte9ThenDecrementGlobalWorkByteBAndAdvanceWorkByte2WhenZeroBmagicMagic00903_801EF814
 *           for the global-byte decrement and the re-read/advance tail,
 *           written here with this span's own guard byte and test direction.
 * @status exact
 * @match 100.00
 * @residual none
 */
void decrementZeroAdvance(
    void) {
  if (g_battle_work[0xB] == 0) {
    D_801EB4E0[0xB]--;
    g_battle_work[2]++;
  }
}
