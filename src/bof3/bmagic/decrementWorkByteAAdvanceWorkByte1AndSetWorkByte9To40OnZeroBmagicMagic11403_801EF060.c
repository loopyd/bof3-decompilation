#include "bof3/bof3.h"

/* Scratchpad work-object pointer cell, bound at 0x1F800044 by this target's map
 * and by the exact siblings of this handler in the bmagic overlay family. It
 * stays a named cell rather than the constant-address memory accessor because
 * the original re-materialises the base at each access (lui $v1,0x1F80 /
 * lw $v1,68($v1)): the accessor's constant address is folded once by CSE into a
 * full-address register and reused across the byte stores. */
extern u8 *g_battle_work; /* @source 0x1F800044 @kind data */

/* @source 0x801EF060
 * @behavior Per-frame handler of the work object published at the scratchpad
 *           pointer cell 0x1F800044. Decrements the object's unsigned byte 0xA
 *           with a byte-width read-modify-write (lbu / addiu -1 / sb) and
 *           compares the stored value with zero (andi 0xFF / bnez). Only when
 *           that byte reaches zero it re-reads the cell, increments the
 *           object's byte 1 by one at byte width, and re-reads the cell once
 *           more to store 0x40 into byte 9. Byte 1 is the state byte this
 *           overlay's step dispatcher at 0x801EF774 indexes its local handler
 *           table with, so the increment is the state advance; the overlay's
 *           byte 9 sits at the same offset the family's other handlers use as
 *           their countdown, and the constant 0x40 is published there. Each read
 *           of the cell is its own lui/lw pair, required because the stores
 *           through the loaded work pointer may alias the cell. The function is
 *           a leaf with no frame; it takes no arguments, returns nothing and
 *           touches no other state. The only in-image reference to 0x801EF060 is
 *           the word at payload offset 0x18, entry 5 of the leading seven-word
 *           handler run (0x801EEDA0, 0x801EEE84, 0x801EEEE4, 0x801EEF44,
 *           0x801EEFFC, 0x801EF060, 0x801EF0BC) that occupies the payload words
 *           at 0x801EEC04 through 0x801EEC1C, so the entry is dispatched and has
 *           no jal caller. Statement order, operand forms and declaration style
 *           follow the exact sibling
 *           decrementWorkByteAAndAdvanceWorkByte1OnZeroBmagicMagic10103_801EEF20,
 *           whose bytes differ in the trailing byte-9 store this span adds.
 * @status exact
 * @match 100.00
 * @residual none
 */
void decrementWorkByteAAdvanceWorkByte1AndSetWorkByte9To40OnZeroBmagicMagic11403_801EF060(void) {
  if (--g_battle_work[0xA] == 0) {
    g_battle_work[1]++;
    g_battle_work[9] = 0x40;
  }
}
