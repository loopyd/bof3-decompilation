#include "bof3/bof3.h"

/* Scratchpad work-object pointer cell. Both accesses go through this symbol
 * rather than a constant address because the arm stores through the loaded
 * pointer when it writes work byte 0x9, which the compiler cannot prove clear
 * of the cell, so the original re-materializes the cell address
 * (lui $v1,0x1F80 / lw $v1,0x44($v1)) and reloads the work object before the
 * byte-2 read. This body is the byte-identical twin of the exact span at
 * 0x801EF4E4 of emi/bmagic/magic073/03, which showed that access form. */
extern u8 *D_1F800044; /* @source 0x1F800044 @kind data */

/* @source 0x801EF4D0
 * @behavior Step of the scratchpad work object that the six-entry pointer run
 *           at 0x801EEC04 publishes in its fourth slot, so the byte-2
 *           dispatcher at 0x801EEFE0 copies it into its frame and invokes it
 *           with no arguments; no jal in this payload names the address. It
 *           loads the work-object pointer cell at 0x1F800044, reads the work
 *           object's byte 0xB and compares it against five with an unsigned
 *           bound (lbu/sltiu), so the arm is taken for every value below five.
 *           Only on that path it stores 8 into the work byte at 0x9, the
 *           countdown the effect submitter at 0x801EF2C8 re-arms, and then
 *           re-reads the cell and increments the work byte at 0x2 by one at
 *           byte width (lbu/addiu 1/sb). Byte 0x2 is the index the byte-2
 *           dispatcher scales by four into that six-entry run, so the arm
 *           advances the work object to its next handler. The guard's cell load
 *           and the byte-9 store share one load because nothing has been
 *           stored through the pointer yet; the byte-2 read-modify-write
 *           re-reads the cell after that store. Takes no arguments, returns
 *           nothing, keeps no frame and reads or writes no other state.
 * @status exact
 * @match 100.00
 * @residual none
 */
void stateByte9Below5Byte2(
    void) {
  if (D_1F800044[0xB] < 5u) {
    D_1F800044[9] = 8;
    D_1F800044[2]++;
  }
}
