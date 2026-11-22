#include "bof3/bof3.h"

/* Scratchpad work-object pointer cell 0x1F800044. The cell is re-evaluated for
 * every access group because a byte store through the loaded pointer may alias
 * it, so the compiler re-materializes `lui $vN,0x1F80` / `lw $vN,0x44($vN)`
 * exactly where the original does (three times in this body). Cell spelling and
 * declaration style follow the exact sibling
 * incrementWorkByte9AndDecrementWorkByteAThenDecrementGlobalWorkByteBAnd
 * InvokeHelperWhenZeroBmagicMagic14503_801EF0A0 (0x801EF0A0), which reaches the
 * same cell in this overlay. */

/* @source 0x801EF034
 * @behavior Entry 2 of this overlay's four-entry handler run at 0x801EF414
 *           (0x801EEE70, 0x801EEEE8, 0x801EF034, 0x801EF0A0), whose base word
 *           0x801EF41C holds this address and which the dispatcher at
 *           0x801EEE2C selects on unsigned byte 0x1 of the scratchpad work
 *           object published at the pointer cell 0x1F800044. It reads that
 *           cell, adds two modulo 256 to the object's unsigned byte 0x09 with
 *           a byte-width read-modify-write (lbu / addiu 2 / sb), re-reads the
 *           cell and adds two modulo 256 to byte 0x0A the same way, then
 *           re-reads the cell once more and tests byte 0x09 against 0x10 for
 *           equality with a plain branch on the loaded byte (lbu / addiu
 *           $v0,$zero,0x10 / bne), so the arm runs only at that value. In the
 *           arm it increments the object's byte 0x2 from the pointer that read
 *           already holds (lbu / addiu 1 / sb). The test re-reads the byte
 *           instead of reusing the value stored two statements earlier. The
 *           function is a leaf with no frame; it takes no arguments, returns
 *           nothing and reads or writes no other state. Bytes
 *           0x801EF034..0x801EF09F are byte-identical to 0x801EF2D4 of
 *           bmagic/magic009/03, already lifted exact as
 *           add2ToWorkBytes9AndAThenAdvanceWorkByte2At16BmagicMagic00903_801EF2D4,
 *           and to 0x801EF520 of bmagic/magic146/03, whose row is still asm;
 *           one payload comparison proved both spans. The clone dry-run verified
 *           the 108 bytes against the magic009/03 representative, and its
 *           statement order, operand forms and reload points were kept while
 *           the cell was respelled with this overlay's sibling.
 * @status exact
 * @match 100.00
 * @residual none
 */
void add2ToWorkBytes9AndAThenAdvanceWorkByte2At16BmagicMagic14503_801EF034(void)
{
  u8** slots;

  slots = SPAD_PTR_TABLE(u8);

  slots[0x11][0x09] += 2;
  slots[0x11][0x0A] += 2;
  if (slots[0x11][0x09] == 0x10) {
    slots[0x11][0x02] += 1;
  }
}
