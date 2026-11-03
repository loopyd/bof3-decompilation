#include "bof3/bof3.h"

/* Scratchpad work-object pointer cell, shared by this overlay's dispatchers and
 * handlers. Declared as the same volatile u8 pointer the exact sibling
 * decrementMotionCountdownAndSubtractMotionPairThenAdvanceWorkByte2
 * BmagicMagic00800_801F04DC uses: every access in the original re-reads the
 * cell, and the named symbol lets the assembler fold the constant address into
 * the load (lui $vN,0x1F80 / lw $vN,68($vN)), which an integer-constant address
 * does not. */
extern u8* volatile g_battle_work; /* @source 0x1F800044 @kind data */

/* Shared battle work pointer in main RAM, outside this overlay's image. The
 * sibling bmagic lifts read the same record through this pointer
 * (incrementWorkByte2WhenGlobalWorkByteBIs84BmagicMagic11303_801EF8A4 tests its
 * byte 0xB, decrementMotionCountdownAndSubtractMotionPairThenAdvanceWorkByte2
 * BmagicMagic00800_801F04DC stores 0xFF there); splat binds the address through
 * this target's generated undefined_syms_auto.txt, so no target-map row is
 * needed. */
extern u8 *D_801EB4E0; /* @source 0x801EB4E0 @kind unknown */

/* @source 0x801F11C4
 * @behavior Entry 4 of the six-entry work-state table at 0x801EEC94
 * (0x801F0E54, 0x801F0FA4, 0x801F1090, 0x801F117C, 0x801F11C4, 0x801F1274),
 * which 0x801F0DA8 dispatches on scratchpad work-object byte 2. Per frame it
 * subtracts 3 from the work object's three colour channels at bytes
 * 0x5D/0x5E/0x5F - the r0/g0/b0 triplet that state entry 0x801F0E54 seeds with
 * 0xB0 and that the scenario lifts copy into SPRT fields - and then reads the
 * channel at 0x5D back through a signed byte view. When that value equals
 * -0x80 (0x80 unsigned, the half-brightness floor) it decrements byte 0xB of
 * the shared battle work record at 0x801EB4E0 and advances work byte 2 to the
 * table's final entry 0x801F1274. Each access re-reads the work-object pointer
 * cell at 0x1F800044, so the cell is loaded five times. Leaf with no frame;
 * returns nothing and reads or writes no other state.
 * @status exact
 * @match 100.00
 * @residual none
 */
void subtractWorkColorTripletBy3ThenAdvanceWorkByte2WhenChannelIs80BmagicMagic00800_801F11C4(void) {
  g_battle_work[0x5D] -= 3;
  g_battle_work[0x5E] -= 3;
  g_battle_work[0x5F] -= 3;
  if (((s8*)g_battle_work)[0x5D] == -0x80) {
    D_801EB4E0[0xB]--;
    g_battle_work[2]++;
  }
}
