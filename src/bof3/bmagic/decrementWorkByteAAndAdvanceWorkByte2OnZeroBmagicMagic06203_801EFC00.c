#include "bof3/bof3.h"

/* Scratchpad work-object pointer cell, bound at 0x1F800044 by this target's
 * map and by every exact sibling of this handler in the same overlay family.
 * It is kept as a named cell rather than the constant-address memory accessor
 * because the original re-materialises the base at each access
 * (lui $v1,0x1F80 / lw $v1,68($v1)); the accessor's constant address let the
 * build hoist one lui/ori base and load through it twice. */
extern u8 *g_battle_work; /* @source 0x1F800044 @kind data */

/* @source 0x801EFC00
 * @behavior Entry 0 of this overlay's work-byte-2 handler table based at
 *           0x801F0C24, whose dispatcher at 0x801EFBBC indexes that table on
 *           the work-object byte at offset 2. Decrements the unsigned byte 0xA
 *           of the scratchpad work object reached through the pointer cell at
 *           0x1F800044 with a byte-width read-modify-write (lbu / addiu -1 /
 *           sb) and compares the stored value with zero (andi 0xFF / bnez).
 *           Only when that byte reaches zero it re-reads the cell and
 *           increments the object's byte 2 with a byte-width add, which moves
 *           the dispatcher to its next handler step. The second read of the
 *           cell is required: the store through the loaded work pointer may
 *           alias the cell. Takes no arguments, returns nothing, keeps no frame
 *           and reads or writes no other state. Bytes
 *           0x801EFC00..0x801EFC4B are byte-identical to the already-exact
 *           sibling decrementWorkByteAAndAdvanceWorkByte2OnZeroBmagicMagic08203_801EFD74,
 *           whose body spelling this follows.
 * @status exact
 * @match 100.00
 * @residual none
 */
void decrementWorkByteAAndAdvanceWorkByte2OnZeroBmagicMagic06203_801EFC00(void) {
  if (--g_battle_work[0xA] == 0) {
    g_battle_work[2]++;
  }
}
