#include "bof3/bof3.h"

/* Scratchpad work-object pointer cell shared by the battle and bmagic
 * overlays. The cell is read once per access, so the original re-materializes
 * its address (lui $vN,0x1F80 / lw $vN,68($vN)) for each of the two accesses. */
extern u8 *g_battle_work; /* @source 0x1F800044 @kind data */

/* Four per-slot byte counters closed by the original's byte loop. The draw
 * code indexes them by slot (function at 0x801EF984 increments one counter per
 * active slot and masks it with 0xF when work byte 2 is 1, the function at
 * 0x801EFCC8 stops at the 0x10 bound) so they stay address-anchored here. */
extern u8 D_801F00B0[]; /* @source 0x801F00B0 @kind data */

/* @source 0x801EF878
 * @behavior Clears the four per-slot byte counters at 0x801F00B0 through
 * 0x801F00B3 with a byte-width store loop over an unsigned index bounded by
 * 4, then clears the scratchpad work-object byte at offset 0x9 and
 * re-reads the work-object pointer cell at 0x1F800044 and increments that
 * object's byte at offset 0x2 with a byte-width add (lbu, addiu $v0,$v0,1,
 * sb scheduled into the $ra delay slot). Nothing is returned, no other state
 * is read or written and the function has no frame. Its trailing 48 bytes are
 * byte-identical to the exact siblings
 * clearWorkByte9AndIncrementWorkByte2BmagicMagic06403_801F0DA8
 * (emi/bmagic/magic064/03@0x801F0DA8) and
 * clearWorkByte9AndIncrementWorkByte2BmagicMagic13103_801F0178
 * (emi/bmagic/magic131/03@0x801F0178), whose two-statement shape supplies the
 * work-byte tail.
 * @status exact
 * @match 100.00
 * @residual none
 */
void clearSlotCountersAndWorkByte9ThenIncrementWorkByte2BmagicMagic06603_801EF878(void) {
  u32 i;

  for (i = 0; i < 4; i++) {
    D_801F00B0[i] = 0;
  }
  g_battle_work[9] = 0;
  g_battle_work[2]++;
}
