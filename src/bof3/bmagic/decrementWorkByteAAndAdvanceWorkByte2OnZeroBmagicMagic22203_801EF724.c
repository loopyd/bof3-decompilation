#include "bof3/bof3.h"

/* Scratchpad work-object pointer cell, bound at 0x1F800044 by this target's map
 * and by every exact sibling of this handler in the same overlay family. It is
 * kept as a named cell rather than the constant-address memory accessor because
 * the original re-materialises the base at each of its two accesses
 * (lui $v1,0x1F80 / lw $v1,68($v1)). */
extern u8 *g_battle_work; /* @source 0x1F800044 @kind data */

/* @source 0x801EF724
 * @behavior Work-object handler step, entry 12 of the sixteen-word
 *           handler-pointer run at 0x801F0D54 whose word at 0x801F0D84 holds
 *           this function address, so the byte-1 dispatcher
 *           dispatchStateByte1Table0d54BmagicMagic22203_801EEEE0 selects it
 *           while work object byte 1 is 12 and no jal in this payload targets
 *           the span. It
 *           reads the scratchpad work object through the pointer cell at
 *           0x1F800044 and decrements its unsigned byte 0xA with a byte-width
 *           read-modify-write (lbu / addiu -1 / sb), comparing the stored byte
 *           with zero (andi 0xFF / bnez). Only when that byte reaches zero it
 *           re-reads the cell and increments the object's byte 2 with a
 *           byte-width add, which moves the byte-2 dispatcher to its next
 *           sub-step. The second read of the cell is required: the store
 *           through the loaded work pointer may alias the cell. Leaf with no
 *           frame; takes no arguments, returns nothing and reads or writes no
 *           other state. The 0x4C-byte body is byte-identical to that of the
 *           exact sibling decrementWorkByteAAndAdvanceWorkByte2OnZeroBmagicMagic11103_801EF288,
 *           whose statement order and operand forms it follows.
 * @status exact
 * @match 100.00
 * @residual none
 */
void decrementWorkByteAAndAdvanceWorkByte2OnZeroBmagicMagic22203_801EF724(void) {
  if (--g_battle_work[0xA] == 0) {
    g_battle_work[2]++;
  }
}
