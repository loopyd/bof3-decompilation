#include "bof3/bof3.h"

/* Scratchpad work-object pointer cell shared by the battle and bmagic overlays;
 * this target's map binds it at 0x1F800044. Each of the body's three reads
 * re-materializes the cell with its own lui $vN,0x1F80 / lw $vN,0x44($vN) pair,
 * because every byte store through the loaded pointer may alias the cell, so
 * the plain named cell below supplies all three reloads by itself. */
extern u8 *g_battle_work; /* @source 0x1F800044 @kind data */

/* @source 0x801EF2D4
 * @behavior Entry 3 of this overlay's five-entry work-byte-2 handler pointer
 *           run based at 0x801EF89C, which the dispatcher at 0x801EF0DC indexes
 *           on the work object's byte at offset 0x2. It reads the scratchpad
 *           work-object pointer cell at 0x1F800044, adds two modulo 256 to the
 *           work object's unsigned byte 0x9 with a byte-width read-modify-write
 *           (lbu / addiu 2 / sb), re-reads the cell and adds two modulo 256 to
 *           byte 0xA the same way, then re-reads the cell once more and tests
 *           the object's byte 0x9 against 0x10 for equality with a plain branch
 *           on the loaded byte (lbu / addiu $v0,$zero,0x10 / bne), so the arm
 *           runs only at that value. In the arm it increments the object's byte
 *           0x2 from the pointer that read already holds (lbu / addiu 1 / sb),
 *           which moves the dispatcher to its next step. The test re-reads the
 *           byte rather than reusing the value it stored two statements earlier.
 *           The function is a leaf with no frame; it takes no arguments, returns
 *           nothing and reads or writes no other state. The word at 0x801EF8A8
 *           holds this address and no jal in this payload reaches it. Bytes
 *           0x801EF2D4..0x801EF33F are byte-identical to the unlifted
 *           0x801EF034 of bmagic/magic145/03 and 0x801EF520 of
 *           bmagic/magic146/03, whose rows are still asm and own no source that
 *           could be cloned, so the clone lever was declined and the statement
 *           order, operand forms and reload points come from the family sibling
 *           incrementWorkByte9Add2ToWorkByteAThenAdvanceWorkByte2At20BmagicMagic
 *           05903_801EF504 and the target-local sibling
 *           incrementByteWhenZero, both
 *           measured exact when this body was written.
 * @status exact
 * @match 100.00
 * @residual none
 */
void workAdvanceAt16(void) {
  g_battle_work[0x9] += 2;
  g_battle_work[0xA] += 2;
  if (g_battle_work[0x9] == 0x10) {
    g_battle_work[2]++;
  }
}
