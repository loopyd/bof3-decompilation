#include "bof3/bof3.h"

/* Scratchpad work-object pointer cell, named the same way the exact siblings on
 * the neighbouring bmagic overlays name it. The symbolic address folds into
 * each load (lui $vN,0x1F80 / lw $vN,68($vN)), so this function's two separate
 * cell reads each re-materialise the base. The row lives in this target's map. */
extern u8 *g_battle_work; /* @source 0x1F800044 @kind data */

/* @source 0x801EF24C
 * @behavior Frame step of the overlay work object: advances the work object's
 *           byte at offset 0x9 by one with a byte-width read-modify-write
 *           (lbu / addiu 1 / sb) and tests the truncated value it just stored
 *           (andi 0xFF of the stored register, no second load of the byte).
 *           Only when that byte reached 0x78 does it read the scratchpad
 *           pointer cell at 0x1F800044 a second time and increment the work
 *           object's state byte at offset 0x2 (lbu / addiu 1 / sb), advancing
 *           the overlay's work byte 2 handler sequence. Leaf with no frame:
 *           takes no arguments, returns nothing and reads or writes no other
 *           state. The stored byte owns the compare, so the cell is read once
 *           for the update and again on the taken path, not once for both. No
 *           in-image jal or pointer word names this address, so it is entered
 *           from outside the payload; its prologue-free head and single
 *           jr $ra tail bound it.
 * @status exact
 * @match 100.00
 * @residual none
 */
void incrementWorkByte9AndAdvanceWorkByte2At78BmagicMagic12203_801EF24C(void) {
  if ((g_battle_work[9] += 1) == 0x78) {
    g_battle_work[2]++;
  }
}
