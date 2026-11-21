#include "bof3/bof3.h"

/* Shared battle work pointer in main RAM, outside this overlay's image. Splat
 * binds the address through this target's generated undefined_syms_auto.txt row
 * D_801EB4E0 = 0x801EB4E0, and the store is byte-addressed, so the local
 * declaration keeps the byte view only. */
extern u8 *D_801EB4E0; /* @source 0x801EB4E0 @kind unknown */

/* Scratchpad work-object pointer cell shared by the battle and bmagic overlays.
 * This target's map binds it at 0x1F800044 as g_battle_work, the same binding
 * the exact siblings decrementWorkByteAAndAdvanceWorkByte2OnZeroBmagicMagic
 * 07803_801EF0E0 and clearWorkByteAAndIncrementWorkByte2WhenGlobalWorkByte
 * BIs2BmagicMagic07803_801EF228 use. The original re-materializes the cell
 * address before each of the two taken-path accesses (lui $v1,0x1F80 / lw
 * $v1,0x44($v1)), so both statements stay on the named cell instead of sharing
 * one loaded base. */
extern u8 *g_battle_work; /* @source 0x1F800044 @kind data */

/* @source 0x801EF1CC
 * @behavior Entry 6 of this overlay's nine-word work-byte-1 handler pointer run
 *           based at 0x801F0878 (its index is 5, verified against the payload
 *           words), so it is reached through that run and has no jal caller.
 *           Decrements the unsigned byte 0xA of the scratchpad work object
 *           reached through the pointer cell at 0x1F800044 as a byte-width
 *           read-modify-write (lbu / addiu -1 / sb) and tests the stored value
 *           against zero (andi 0xFF / bnez). Only when that byte reaches zero
 *           it loads the shared battle work pointer at 0x801EB4E0 and stores
 *           one into that record's byte 0xB, then re-materializes the
 *           scratchpad cell with its own lui/lw pair and increments the work
 *           object's byte 2 at byte width (lbu / addiu 1 / sb), which moves the
 *           dispatcher to its next step. The constant one is materialized in
 *           the branch delay slot, ahead of the record load. Takes no
 *           arguments, returns nothing, keeps no frame and reads or writes no
 *           other state. The decrement guard through the same cell, its 36-byte
 *           opening, is byte-identical to the exact same-target sibling
 *           decrementWorkByteAAndAdvanceWorkByte2OnZeroBmagicMagic07803_
 *           801EF0E0, and the record store follows the byte-0xB write shape of
 *           the exact sibling clearWorkByteAAndIncrementWorkByte2WhenGlobalWork
 *           ByteBIs2BmagicMagic07803_801EF228. No payload under out/binaries
 *           holds this 92-byte body elsewhere, so the clone lever is
 *           ineligible.
 * @status exact
 * @match 100.00
 * @residual none
 */
void decrementWorkByteAThenSetGlobalWorkByteBTo1AndAdvanceWorkByte2OnZeroBmagicMagic07803_801EF1CC(void) {
  if (--g_battle_work[0xA] == 0) {
    D_801EB4E0[0xB] = 1;
    g_battle_work[2]++;
  }
}
