#include "bof3/bof3.h"

/* Work-object pointer cell of the shared main executable scratchpad. The
 * compiled original re-materializes and reloads the cell (lui $v1,0x1F80 / lw
 * $v1,0x44($v1)) before the arm's last store, because the byte store above it
 * may alias the word, so the cell is reached through this symbol rather than
 * through a constant address, which the compiler materializes once and reuses.
 * No map row of this target names the word, so it stays address-anchored; the
 * sibling overlays that bind it to a semantic name do so in their own maps. */
extern u8 *D_1F800044; /* @source 0x1F800044 @kind data */

/* @source 0x801EF158
 * @behavior Step of the scratchpad work object that this overlay entry
 *           dispatcher func_801EEC28 runs. It reads the work-object pointer cell
 *           at 0x1F800044 once, loads the work object byte 0xB and compares it
 *           against five with an unsigned bound (lbu/sltiu), so the arm is taken
 *           for every value below five. Only on that path it stores 8 into the
 *           work byte at 0x9 - the countdown the handler func_801EEFA0
 *           decrements at 0x801EF010 and re-arms to 8 at 0x801EF014 when it
 *           reaches zero - and then re-reads the cell and increments the work
 *           byte at 0x1, the stage index, by one at byte width
 *           (lbu/addiu 1/sb). The entry dispatcher copies the six words at
 *           0x801EEC04 into its frame and indexes them with that byte scaled by
 *           four; this address is word 3 of that run, so the arm advances the
 *           stage to word 4. Takes no arguments, returns nothing, keeps no
 *           frame and reads or writes no other state. The 0x48-byte span
 *           0x801EF158..0x801EF19F compares equal to 72 bytes of
 *           emi/bmagic/magic162/03 at the same address, whose own lift recorded
 *           an exact match and supplied the statement order and the name, and to
 *           emi/bmagic/magic071/03 at 0x801EF15C, both re-proved against this
 *           payload scan rather than copied from a comment.
 * @status exact
 * @match 100.00
 * @residual none
 */
void setWorkByte9To8AndIncrementWorkByte1WhenWorkByteBBelow5BmagicMagic16803_801EF158(
    void) {
  if (D_1F800044[0xB] < 5u) {
    D_1F800044[9] = 8;
    D_1F800044[1]++;
  }
}
