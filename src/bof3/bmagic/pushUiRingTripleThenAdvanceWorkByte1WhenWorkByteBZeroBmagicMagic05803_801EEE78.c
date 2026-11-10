#include "bof3/bof3.h"

/* Scratchpad work-object pointer cell at 0x1F800044, bound by this target map
 * row g_battle_work. The compiled original materializes the cell address and
 * reloads the pointer again after the pushed triple, so the cell is reached
 * through this symbol rather than through a constant address, which the
 * compiler materializes once and reuses. The exact siblings
 * setWorkByte9To8AndIncrementWorkByte1WhenWorkByteBZeroBmagicMagic07603_801EF11C
 * and selectWorkRecordClearFlag40ThenAdvanceWorkByte1AndClearWorkBytes2And3BmagicMagic15103_801EF2A0
 * declare the same cell this way. */
extern u8 *g_battle_work; /* @source 0x1F800044 @kind data */

/* Texture-handle lookup in the shared main executable, outside this overlay
 * image: it returns the texture handle for the given colour index, a role
 * include/bof3/ui/sisyou00_internal.h records for this address and leaves
 * unlifted. This body passes the colour index 0x14. */
extern u32 func_801502D0(u32 arg0); /* @source 0x801502D0 */

/* UI ring producer in the shared main executable, outside this overlay image:
 * it appends one (byte, byte, word) triple into the 16-entry UI ring and
 * advances the ring tail. The battle overlay binds this address as
 * pushUiRingTriple from its own lift src/bof3/battle/pushUiRingTriple.c, which
 * was measured exact when this seed was written. */
extern void pushUiRingTriple(s8 arg0, s8 arg1, u32 arg2); /* @source 0x801DE8C0 */

/* @source 0x801EEE78
 * @behavior Overlay step body: word 1 of the nine-word step-handler table the
 *           payload header holds at 0x801EEC04, which the byte-2 step
 *           dispatcher reads, so no jal in this payload reaches it. It loads
 *           the scratchpad work-object pointer cell at 0x1F800044 once and
 *           tests the work object byte at offset 0x0B against zero with a plain
 *           branch on the loaded byte (lbu/bnez), so the arm runs only when
 *           that byte is zero. In the arm it calls the texture-handle lookup at
 *           0x801502D0 with the colour index 0x14 and pushes one UI ring triple
 *           (1, 0x3C, that handle) through the producer at 0x801DE8C0. It then
 *           re-reads the cell, because the push may alias it, and increments
 *           the work byte at offset 0x1 by one at byte width (lbu, addiu
 *           $v0,$v0,1, sb). It keeps the return address at 0x10 in a 0x18-byte
 *           frame, takes no arguments and returns nothing.
 * @status exact
 * @match 100.00
 * @residual none
 */
void pushUiRingTripleThenAdvanceWorkByte1WhenWorkByteBZeroBmagicMagic05803_801EEE78(
    void) {
  if (g_battle_work[0xB] == 0) {
    pushUiRingTriple(1, 0x3C, func_801502D0(0x14));
    g_battle_work[1]++;
  }
}
