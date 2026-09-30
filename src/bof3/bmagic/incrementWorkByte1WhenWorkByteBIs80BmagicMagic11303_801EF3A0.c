#include "bof3/bof3.h"

/* @source 0x801EF3A0
 * @behavior Reads the scratchpad work-object pointer cell at 0x1F800044 once
 * and, only when the work object's byte at offset 0xB equals 0x80, increments
 * the byte at offset 0x1 with a byte-width add (the original loads it with lbu,
 * adds 1 and stores it back with sb). Nothing is returned and no other state is
 * read or written. The pointer cell stays in $a0 across the guard so both the
 * guard byte and the incremented byte come from one load of 0x1F800044; this is
 * the same guard shape as the sibling
 * incrementWorkByte2WhenWorkByteBZeroBmagicMagic15103_801F00B4, which tests
 * that same work byte 0xB against zero.
 * @status exact
 * @match 100.00
 * @residual none
 */
void incrementWorkByte1WhenWorkByteBIs80BmagicMagic11303_801EF3A0(void) {
  u8 *work = SPAD_PTR_SLOT(u8, 0x44u);

  if (work[0xB] == 0x80) {
    work[1]++;
  }
}
