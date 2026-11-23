#include "bof3/bof3.h"

/* Scratchpad work-object pointer cell 0x1F800044. The cell is re-evaluated for
 * every access group because a store through the loaded pointer may alias it, so
 * the compiler re-materializes `lui 0x1F80` / `lw 0x44` exactly where the
 * original does. Statement order, cell spelling and declaration style follow the
 * exact sibling incrementPairAdvance
 * (0x801EEE3C), which reaches the same cell through SPAD_PTR_TABLE; this body
 * carries its own stored bytes, its own -2 step and its own 0x10 threshold. The
 * guard test reads the value the byte 0x09 store just took instead of re-reading
 * that byte, which is why the original masks the stored register with `andi
 * 0xFF` and re-loads the cell only inside the arm - the shape an exact sibling
 * in magic069 0x801EF1D0 measured with the same joined assignment-and-test. */

/* @source 0x801EF09C
 * @behavior Step handler of the scratchpad work object read through pointer cell
 *           0x1F800044 and the second word of the four-word code-pointer run at
 *           0x801EFC64 (0x801EEFFC, 0x801EF09C, 0x801EF124, 0x801EF1B8); no
 *           instruction in this payload loads that run, so its dispatcher lies
 *           outside the overlay image. It increments work byte 0x0B by one,
 *           adds two to work byte 0x0A and subtracts two from work byte 0x09,
 *           each at byte width (lbu / addiu / sb) with the pointer cell
 *           re-read before the group. It then tests the eight-bit value the byte
 *           0x09 store just took (`andi 0xFF`, not a re-read) against 0x10 and,
 *           on equality, re-reads the cell and increments work byte 0x01 - the
 *           run index the in-payload dispatcher at 0x801EEC10 scales by four to
 *           pick one of the three words at 0x801EEC04. So the pair of counts
 *           0x0A and 0x09 ramp in opposite directions until byte 0x09 reaches
 *           0x10 and the step advances. Leaf with no frame; takes no arguments,
 *           returns nothing through the jr $ra at 0x801EF120 and touches no
 *           other state. The roles of work bytes 0x0A, 0x0B and 0x01 beyond
 *           these updates are not established, so they stay offset-anchored.
 * @status exact
 * @match 100.00
 * @residual none
 */
void incrementTwoDecrementTwo(void)
{
  u8** slots;

  slots = SPAD_PTR_TABLE(u8);

  slots[0x11][0x0B] += 1;
  slots[0x11][0x0A] += 2;
  if ((slots[0x11][0x09] -= 2) == 0x10) {
    slots[0x11][0x01] += 1;
  }
}
