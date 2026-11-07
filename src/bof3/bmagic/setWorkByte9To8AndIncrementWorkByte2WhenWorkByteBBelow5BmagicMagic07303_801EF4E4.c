#include "bof3/bof3.h"

/* Scratchpad work-object pointer cell. The compiled original re-materializes and
 * reloads the cell (lui $v1,0x1F80 / lw $v1,0x44($v1)) once after the arm's
 * byte-9 store, because that store through the loaded pointer may alias the
 * cell, so the cell is reached through this address-derived symbol rather than
 * through a constant address, which the compiler materializes once and reuses. */
extern u8 *D_1F800044; /* @source 0x1F800044 @kind data */

/* @source 0x801EF4E4
 * @behavior Step of the scratchpad work object that this overlay's entry
 *           dispatcher func_801EEC28 runs: it loads the work-object pointer cell
 *           at 0x1F800044, reads the work object's byte 0xB and compares it
 *           against five with an unsigned bound (lbu/sltiu), so the arm is taken
 *           for every value below five. Only on that path it stores 8 into the
 *           work byte at 0x9 - the countdown func_801EF06C re-arms to 8 at
 *           0x801EF14C - and then re-reads the cell and increments the work byte
 *           at 0x2 by one at byte width (lbu/addiu 1/sb). Byte 0x2 is the
 *           handler index func_801EEFF4 scales by four into the six-entry run at
 *           0x801EEC04, which it first copies to its frame, so the arm advances
 *           the work object to the next handler. The guard's cell load and the
 *           byte-9 store's cell load are one load, because nothing has been
 *           stored through the pointer yet; the byte-2 read-modify-write
 *           re-reads the cell after that store. Takes no arguments, returns
 *           nothing, keeps no frame and reads or writes no other state. The
 *           0x48-byte body is byte-identical to the unlifted 0x801EF4D0 of
 *           emi/bmagic/magic074/03 and 0x801EF640 of emi/bmagic/magic169/03,
 *           and the same shape with the bound lowered from five is
 *           setWorkByte9To8AndIncrementWorkByte1WhenWorkByteBBelow2Bmagic
 *           Magic06203_801EF078.
 * @status exact
 * @match 100.00
 * @residual none
 */
void setWorkByte9To8AndIncrementWorkByte2WhenWorkByteBBelow5BmagicMagic07303_801EF4E4(
    void) {
  if (D_1F800044[0xB] < 5u) {
    D_1F800044[9] = 8;
    D_1F800044[2]++;
  }
}
