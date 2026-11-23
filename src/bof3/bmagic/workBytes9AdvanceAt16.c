#include "bof3/bof3.h"

/* Scratchpad work-object pointer cell shared by the battle and bmagic overlays;
 * this target's map binds it at 0x1F800044. The cell is re-evaluated for every
 * access group because a byte store through the loaded pointer may alias it, so
 * the original re-materializes `lui $vN,0x1F80` / `lw $vN,0x44($vN)` exactly
 * three times, at 0x801EF520 ($v1), 0x801EF53C ($v1) and 0x801EF558 ($a0), and
 * the plain named cell below supplies all three reloads by itself. The spelling
 * and declaration style follow this target's exact sibling
 * incrementWorkByte9AndDecrementWorkByteAThenDecrementGlobalWorkByteBAnd
 * ClearWorkBytesWhenZeroBmagicMagic14603_801EF58C, which reaches the same cell
 * in this overlay. */
extern u8 *g_battle_work; /* @source 0x1F800044 @kind data */

/* @source 0x801EF520
 * @behavior Entry 2 of this overlay's four-entry work-byte-1 handler pointer
 *           run based at 0x801EF9C0 (0x801EF340, 0x801EF3B8, 0x801EF520,
 *           0x801EF58C), whose word at 0x801EF9C8 holds this address and which
 *           the dispatcher at 0x801EF2FC indexes on unsigned byte 0x1 of the
 *           work object published at the scratchpad pointer cell 0x1F800044. It
 *           reads that cell, adds two modulo 256 to the object's unsigned byte
 *           0x09 with a byte-width read-modify-write (lbu / addiu 2 / sb),
 *           re-reads the cell and adds two modulo 256 to byte 0x0A the same way,
 *           then re-reads the cell a third time and tests byte 0x09 against 0x10
 *           for equality with a plain branch on the loaded byte (lbu / addiu
 *           $v0,$zero,0x10 / bne), so the arm runs only at that value; the test
 *           re-reads the byte instead of reusing the value stored two statements
 *           earlier. In the arm it increments the object's byte 0x02 by one from
 *           the pointer the third read already holds (lbu / addiu 1 / sb). The
 *           function is a leaf with no frame and no saved registers: it takes no
 *           arguments, returns nothing through the jr $ra at 0x801EF588 and
 *           reads or writes no other state, so the roles of work-object bytes
 *           0x09, 0x0A and 0x02 stay offset-anchored. No jal in this payload
 *           reaches it. Bytes 0x801EF520..0x801EF58B are byte-identical to
 *           0x801EF2D4 of emi/bmagic/magic009/03 and 0x801EF034 of
 *           emi/bmagic/magic145/03, and one payload comparison proved both
 *           spans. Both are already lifted exact as
 *           add2ToWorkBytes9AndAThenAdvanceWorkByte2At16BmagicMagic00903_801EF2D4
 *           and add2ToWorkBytes9AndAThenAdvanceWorkByte2At16BmagicMagic14503_801EF034,
 *           so the clone lever applied: this body keeps that representative's
 *           statement order, operand forms and reload points, and the name comes
 *           from those two twin rows.
 * @status exact
 * @match 100.00
 * @residual none
 */
void workBytes9AdvanceAt16(void) {
  g_battle_work[0x9] += 2;
  g_battle_work[0xA] += 2;
  if (g_battle_work[0x9] == 0x10) {
    g_battle_work[2]++;
  }
}
