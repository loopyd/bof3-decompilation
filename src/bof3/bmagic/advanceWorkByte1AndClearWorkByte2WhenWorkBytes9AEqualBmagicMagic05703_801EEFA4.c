#include "bof3/bof3.h"

/* The scratchpad work-object pointer cell 0x1F800044 is re-evaluated for the
 * store that follows a byte store through the loaded pointer, because that
 * store may alias the cell. Statement order, cell spelling and declaration
 * style follow the exact sibling
 * clearWorkByte9SelectWorkState1AndShade0FWorld04Area18813_801F2F30
 * (0x801F2F30), whose guard-loaded pointer stays in one register while a later
 * group re-reads the same cell. */

/* @source 0x801EEFA4
 * @behavior Overlay handler of the scratchpad work object published through the
 *           pointer cell at 0x1F800044. It loads the cell once and compares the
 *           object's unsigned byte 0x09 with its unsigned byte 0x0A; the two
 *           loads share that one pointer. When the two bytes are equal it
 *           increments work byte 0x01 by one at byte width (lbu / addiu 1 / sb)
 *           using the pointer the comparison already holds, so that store is
 *           the only work the arm does through it. The arm then re-reads the
 *           cell, because the byte store may alias it, and clears work byte
 *           0x02 through the reloaded pointer. Byte 0x01 is the work state that
 *           selects the overlay handler table and byte 0x02 is the handler index
 *           that state 1 indexes its own three-word table with, as the sibling
 *           dispatch in this overlay shows; byte 0x09 and byte 0x0A stay
 *           offset-anchored. Leaf with no frame: it takes no arguments, makes no
 *           call and returns nothing. This 0x48-byte span is word 6 of the
 *           nineteen-word code pointer run at 0x801EEC04.
 * @status exact
 * @match 100.00
 * @residual none
 */
void advanceWorkByte1AndClearWorkByte2WhenWorkBytes9AEqualBmagicMagic05703_801EEFA4(void) {
  u8** slots;
  u8* work;

  slots = SPAD_PTR_TABLE(u8);
  work = slots[0x11];
  if (work[9] == work[0xA]) {
    work[1]++;
    slots[0x11][2] = 0;
  }
}
