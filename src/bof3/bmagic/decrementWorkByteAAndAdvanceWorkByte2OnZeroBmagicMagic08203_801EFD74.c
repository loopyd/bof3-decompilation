#include "bof3/bof3.h"

/* Scratchpad work-object pointer cell, bound at 0x1F800044 by this target's map
 * and by every exact sibling of this handler in the same overlay family. It is
 * kept as a named cell rather than the constant-address memory accessor
 * because the original re-materialises the base at each access
 * (lui $v1,0x1F80 / lw $v1,68($v1)); the accessor's constant address let the
 * build hoist one lui/ori base and load through it twice, which measured
 * 15/19 instructions at the same byte count. */
extern u8 *g_battle_work; /* @source 0x1F800044 @kind data */

/* @source 0x801EFD74
 * @behavior Per-frame step of the work-object handler this overlay dispatches
 * on work-object byte 2: decrements the unsigned byte 0xA of the scratchpad
 * work object reached through the pointer cell at 0x1F800044 with a byte-width
 * read-modify-write (lbu / addiu -1 / sb) and compares the stored value with
 * zero (andi 0xFF / bnez). Only when that byte reaches zero it re-reads the
 * cell and increments the object's byte 2 with a byte-width add, advancing the
 * dispatcher at 0x801EFD30 to the next byte-2 handler step. The second read of
 * the cell is required: the store through the loaded work pointer may alias
 * the cell. Takes no arguments, returns nothing, keeps no frame and reads or
 * writes no other state. Bytes 0x801EFD74..0x801EFDBF are the first entry of
 * the handler table based at 0x801F0D28 and are byte-identical to the sibling
 * decrementWorkByteAAndAdvanceWorkByte2OnZeroBmagicMagic13103_801F0264, whose
 * body spelling this follows.
 * @status exact
 * @match 100.00
 * @residual none
 */
void decrementWorkByteAAndAdvanceWorkByte2OnZeroBmagicMagic08203_801EFD74(void) {
  if (--g_battle_work[0xA] == 0) {
    g_battle_work[2]++;
  }
}
