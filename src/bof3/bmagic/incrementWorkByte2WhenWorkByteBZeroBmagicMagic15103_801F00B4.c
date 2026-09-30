#include "bof3/bof3.h"

/* @source 0x801F00B4
 * @behavior Reads the scratchpad work-object pointer cell at 0x1F800044 once
 * and, only when the work object's byte at offset 0xB is zero, increments the
 * byte at offset 0x2 with a byte-width add (the original loads it with lbu,
 * adds 1 and stores it back with sb). Nothing is returned and no other state is
 * read or written; the sibling increments the same offset 0x2 byte
 * unconditionally (incrementWorkByte2BmagicMagic06403_801EF87C).
 * @status exact
 * @match 100.00
 * @residual none
 */
void incrementWorkByte2WhenWorkByteBZeroBmagicMagic15103_801F00B4(void) {
  u8 *work = SPAD_PTR_SLOT(u8, 0x44u);

  if (work[0xB] == 0) {
    work[2]++;
  }
}
