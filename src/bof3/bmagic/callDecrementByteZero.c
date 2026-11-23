#include "bof3/bof3.h"

/* Local helper at 0x801F08AC. Its own entry takes no arguments and keeps a
 * 0x20-byte frame; this caller discards its result. */
extern void func_801F08AC(void); /* @source 0x801F08AC @kind function */
extern u8 *g_battle_work; /* @source 0x1F800044 @kind data */

/* @source 0x801F01C0
 * @behavior Invokes the target-local helper at 0x801F08AC, then decrements
 *           work-object byte 0xA modulo 256 through the scratchpad pointer
 *           cell at 0x1F800044 with a byte-width read-modify-write
 *           (lbu / addiu -1 / sb) and compares the stored value with zero
 *           (andi 0xFF / bnez) without reloading the byte. Only when that
 *           byte reaches zero it re-reads the cell and increments the
 *           object's byte 2 modulo 256. The second read of the cell is
 *           required, because the store through the loaded work pointer may
 *           alias the cell. Takes no arguments and returns nothing; its
 *           0x18-byte frame exists only to keep the return address across
 *           the call. No in-payload jal and no in-payload pointer word names
 *           this span, so nothing inside the payload calls it. The
 *           statement order, operand forms and declaration style follow the
 *           family body decrementWorkByteAAndAdvanceWorkByte2OnZeroBmagicMagic13103_801F0264,
 *           whose byte span this body repeats after the leading call; a
 *           word-masked scan of the shipped payloads found no byte-identical
 *           twin of this span.
 * @status exact
 * @match 100.00
 * @residual none
 */
void callDecrementByteZero(
    void) {
  func_801F08AC();
  if (--g_battle_work[0xA] == 0) {
    g_battle_work[2]++;
  }
}
