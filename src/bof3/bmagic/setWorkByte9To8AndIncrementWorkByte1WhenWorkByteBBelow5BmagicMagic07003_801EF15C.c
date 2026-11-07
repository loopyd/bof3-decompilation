#include "bof3/bof3.h"

/* Scratchpad work-object pointer cell. The compiled original re-materializes and
 * reloads the cell (lui $v1,0x1F80 / lw $v1,0x44($v1)) before the arm's last
 * store, so the cell is reached through this address-derived symbol rather than
 * through a constant address, which the compiler materializes once and reuses. */
extern u8 *D_1F800044; /* @source 0x1F800044 @kind data */

/* @source 0x801EF15C
 * @behavior Step of the scratchpad work object that this overlay's entry
 *           dispatcher func_801EEC28 runs: it reads the work-object pointer cell
 *           at 0x1F800044 once, loads the work object's byte 0xB and compares it
 *           against five with an unsigned bound (lbu/sltiu), so the arm is taken
 *           for every value below five. Only on that path it stores 8 into the
 *           work byte at 0x9 - the countdown func_801EF04C decrements and
 *           re-arms to 8 at 0x801EF118 - and then re-reads the cell and
 *           increments the work byte at 0x1, the stage index the entry
 *           dispatcher indexes the six-word run at 0x801EEC04 with, by one at
 *           byte width (lbu/addiu 1/sb). This address is word 3 of that run, so
 *           the arm advances the stage to word 4. Takes no arguments, returns
 *           nothing, keeps no frame and reads or writes no other state. The
 *           0x48-byte span 0x801EF15C..0x801EF1A3 is byte-identical to the
 *           unlifted 0x801EF15C of emi/bmagic/magic071/03 and magic072/03 and to
 *           the unlifted 0x801EF158 of emi/bmagic/magic162/03 and magic168/03,
 *           and its body has the same shape as
 *           setWorkByte9To8AndIncrementWorkByte1WhenWorkByteBBelow2Bmagic
 *           Magic06203_801EF078, which differs only in that bound.
 * @status exact
 * @match 100.00
 * @residual none
 */
void setWorkByte9To8AndIncrementWorkByte1WhenWorkByteBBelow5BmagicMagic07003_801EF15C(
    void) {
  if (D_1F800044[0xB] < 5u) {
    D_1F800044[9] = 8;
    D_1F800044[1]++;
  }
}
