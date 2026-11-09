#include "bof3/bof3.h"

/* Scratchpad work-object pointer cell 0x1F800044, bound by this target's map row
 * g_battle_work. Declared as the named cell the exact siblings on this target
 * use: the symbolic address folds into each load (lui $vN,0x1F80 / lw $vN,
 * 68($vN)), and the original re-reads the cell before each store group because
 * a store through the pointer the cell supplied may alias the cell. */
extern u8 *g_battle_work; /* @source 0x1F800044 @kind data */

/* @source 0x801EF1DC
 * @behavior Entry 10 of this overlay's fourteen-entry work-byte-1 handler table
 *           based at 0x801F02A4, whose word 0x801F02CC is the only in-image
 *           reference to this address, so the body has no jal caller. The
 *           dispatcher at 0x801EED98 indexes that table with the scratchpad
 *           work object's state byte 0x1. The body reads the work-object
 *           pointer cell at 0x1F800044 once and loads two fields through it in
 *           one group: the unsigned countdown byte 0x9 (lbu) and the unsigned
 *           halfword 0x3E (lhu), the field the reviewed bmagic siblings treat as
 *           the work distance. It subtracts 1 from the countdown and 0x40 from
 *           the distance, stores both back at their original widths (sb for byte
 *           0x9, then sh for halfword 0x3E, which the original places in the
 *           branch delay slot so it executes on both paths), masks the stored
 *           countdown with andi 0xFF and returns while that byte is not zero.
 *           Only when the countdown reaches zero does it re-read the cell and
 *           store 0x20 into work byte 0x9, then re-read the cell again and
 *           increment work byte 0x2 at byte width, advancing the dispatcher on
 *           work byte 2 to the next handler step. Both re-reads are required:
 *           each store goes through the pointer the cell supplied. Leaf with no
 *           frame; takes no arguments, returns nothing and reads or writes no
 *           other state. The nearest reviewed siblings on other bmagic targets,
 *           decrementWorkByte9AndAdvanceWorkByte2OnZeroBmagicMagic13003_801EF194
 *           and decrementWorkDistanceBy60AdvanceWorkBytes9AndAUpTo10Then...
 *           BmagicMagic15803_801EF140, share this countdown-byte-0x9 shape and
 *           the distance-halfword-0x3E spelling but differ in immediates and in
 *           the zero arm, so this body is authored directly rather than cloned.
 * @status exact
 * @match 100.00
 * @residual none
 */
void decrementWorkByte9Subtract40FromWorkDistanceThenResetByte9To20AndAdvanceWorkByte2OnZeroBmagicMagic07903_801EF1DC(void) {
  u8 *work;
  u8 countdown;

  work = g_battle_work;
  countdown = work[9] - 1;
  *(u16 *)(work + 0x3E) -= 0x40;
  work[9] = countdown;
  if (countdown == 0) {
    g_battle_work[9] = 0x20;
    g_battle_work[2]++;
  }
}
