#include "bof3/bof3.h"

/* Shared battle work pointer in main RAM, outside this overlay's image. The
 * exact siblings decrementWorkByte9ThenSetGlobalWorkByteBToOneAndAdvanceWorkByte2
 * WhenZeroBmagicMagic15103_801EFD24 and incrementWorkByte2WhenGlobalWorkByte1Is2
 * BmagicMagic15103_801EFD80 read the same cell, and this target's generated
 * undefined_syms_auto.txt binds the address, so no target-map row or
 * WEAK_SYMBOL_AT binding is needed. */
extern u8 *D_801EB4E0; /* @source 0x801EB4E0 @kind unknown */

/* Scratchpad work-object pointer cell, bound at 0x1F800044 by this target's
 * map under the name the exact bmagic siblings use. The symbolic address folds
 * into each load (lui $vN,0x1F80 / lw $vN,68($vN)), so the constant-address
 * macro spelling of the same cell would emit lui+ori+lw 0 instead and lose the
 * original's folded displacement. */
extern u8 *g_battle_work; /* @source 0x1F800044 @kind data */

/* @source 0x801F0070
 * @behavior First handler step of this overlay's work sequence. Reads the
 *           shared battle work pointer at 0x801EB4E0 once and, only while that
 *           record's byte at offset 0x1 equals 3 (the constant is
 *           materialized in the branch, not in the load delay), reads the
 *           scratchpad work-object pointer cell at 0x1F800044 once and
 *           decrements the object's byte at offset 0x9 with a byte-width
 *           read-modify-write (lbu / addiu -1 / sb), then tests the truncated
 *           byte it just stored (andi 0xFF of the stored register, no second
 *           load of the cell). Only when that byte reached zero does it
 *           re-read the cell and increment the work object's byte at offset
 *           0x1 with a byte-width add. That cell read is required: the store
 *           through the loaded work pointer may alias the pointer cell. Byte
 *           0x1 is the step index of the dispatcher at 0x801F0018, which
 *           scales it by four and calls entry 3 of the two-entry code pointer
 *           table at 0x801F0654 (entry 0 = 0x801F0070, entry 1 = 0x801F00D8),
 *           so the increment advances the sequence to its second step. Leaf
 *           with no frame; takes no arguments, returns nothing and reads or
 *           writes no other state. Statement order, operand forms and
 *           declaration style follow the exact sibling
 *           decrementWorkByte9ThenSetGlobalWorkByteBToOneAndAdvanceWorkByte2
 *           WhenZeroBmagicMagic15103_801EFD24, whose body differs in this
 *           function's guard and in the offset-0x1 tail.
 * @status exact
 * @match 100.00
 * @residual none
 */
void decrementWorkByte9WhenGlobalWorkByte1Is3ThenAdvanceWorkByte1WhenZeroBmagicMagic09503_801F0070(void) {
  u8 *work;
  u8 countdown;

  if (D_801EB4E0[1] == 3) {
    work = g_battle_work;
    countdown = work[9] - 1;
    work[9] = countdown;
    if (countdown == 0) {
      g_battle_work[1]++;
    }
  }
}
