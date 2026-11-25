#include "bof3/bof3.h"

/* Scratchpad work-object pointer cell of the shared main executable, bound by
 * this target's map row. The compiled original re-materializes and reloads the
 * cell (lui $v1,0x1F80 / lw $v1,0x44($v1)) before the arm's last store, because
 * the byte store above it may alias the word, so the cell is reached through
 * this symbol rather than through a constant address, which the compiler
 * materializes once and reuses. */
extern u8 *g_battle_work; /* @source 0x1F800044 @kind data */

/* @source 0x801EF158
 * @behavior Step of the scratchpad work object that this overlay's entry
 *           dispatcher func_801EEC28 runs: it reads the work-object pointer cell
 *           at 0x1F800044 once, loads the work object's byte 0xB and compares it
 *           against five with an unsigned bound (lbu/sltiu), so the arm is taken
 *           for every value below five. Only on that path it stores 8 into the
 *           work byte at 0x9 - the countdown the overlay's lifted 0x801EEFA0
 *           source decrements and re-arms to 8 when it reaches zero - and then
 *           re-reads the cell and increments the work byte at 0x1, the stage
 *           index the entry dispatcher copies the six words at 0x801EEC04 into
 *           its frame and indexes with that byte scaled by four, by one at byte
 *           width (lbu/addiu 1/sb). This address is word 3 of that run, so the
 *           arm advances the stage to word 4. Takes no arguments, returns
 *           nothing, keeps no frame and reads or writes no other state. The
 *           0x48-byte span 0x801EF158..0x801EF19F compares equal to 72 bytes of
 *           emi/bmagic/magic070/03 at 0x801EF15C - whose own lift measured exact
 *           and supplied the statement order and the name - and to
 *           emi/bmagic/magic071/03 at 0x801EF15C, emi/bmagic/magic072/03 at
 *           0x801EF15C and emi/bmagic/magic168/03 at 0x801EF158, each proved
 *           against this payload scan rather than copied from a comment.
 * @status exact
 * @match 100.00
 * @residual none
 */
void stateSetByte9Advance(
    void) {
  if (g_battle_work[0xB] < 5u) {
    g_battle_work[9] = 8;
    g_battle_work[1]++;
  }
}
