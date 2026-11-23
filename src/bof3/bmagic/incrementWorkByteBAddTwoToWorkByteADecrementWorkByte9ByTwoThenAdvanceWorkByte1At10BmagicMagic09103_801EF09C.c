#include "bof3/bof3.h"

/* Step handler of the scratchpad work object read through pointer cell
 * 0x1F800044, reached as SPAD_PTR_TABLE(u8)[0x11]. The cell is re-evaluated for
 * every access group because a byte store through the loaded pointer may alias
 * it, so the compiler re-materializes `lui 0x1F80` / `lw 0x44` exactly where the
 * original does.
 *
 * The body is byte-identical to the already-exact sibling at the same address in
 * emi/bmagic/magic055/03 (0x801EF09C): the 0x88-byte span compared equal word for
 * word, so its statement order, cell spelling and declaration style carry over
 * with this target's own literals, which are the same literals. The guard test
 * reads the value the byte 0x09 store just took instead of re-reading that byte,
 * which is why the original masks the stored register with `andi 0xFF` and
 * re-loads the cell only inside the arm.
 *
 * This address is the second word of the four-word code-pointer run at
 * 0x801EFC64 (0x801EEFFC, 0x801EF09C, 0x801EF124, 0x801EF1B8), declared in this
 * target as workHandlerAddresses. No instruction in this
 * payload loads that run and no word in the payload spells its base, so the
 * dispatcher that consumes the run lies outside the overlay image; the entry
 * roles stay structural.
 *
 * @source 0x801EF09C
 * @behavior Increments work byte 0x0B by one, adds two to work byte 0x0A and
 *           subtracts two from work byte 0x09, each at byte width (lbu / addiu
 *           / sb) with the pointer cell re-read before the group, then tests the
 *           eight-bit value the byte 0x09 store just took (`andi 0xFF`, not a
 *           re-read of the byte) against 0x10 and, on equality, re-reads the
 *           cell and increments work byte 0x01 by one. Byte 0x01 is the slot
 *           index the in-payload dispatcher at 0x801EEC10 scales by four to pick
 *           one of the three words of the run at 0x801EEC04, so the counts at
 *           0x0A and 0x09 ramp in opposite directions until byte 0x09 reaches
 *           0x10 and the step advances. Leaf with no frame; takes no arguments,
 *           returns nothing through the jr $ra at 0x801EF120 and touches no
 *           other state. The roles of work bytes 0x0A, 0x0B and 0x01 beyond these
 *           updates are not established, so they stay offset-anchored.
 * @status exact
 * @match 100.00
 * @residual none
 */
void incrementWorkByteBAddTwoToWorkByteADecrementWorkByte9ByTwoThenAdvanceWorkByte1At10BmagicMagic09103_801EF09C(void)
{
  u8** slots;

  slots = SPAD_PTR_TABLE(u8);

  slots[0x11][0x0B] += 1;
  slots[0x11][0x0A] += 2;
  if ((slots[0x11][0x09] -= 2) == 0x10) {
    slots[0x11][0x01] += 1;
  }
}
