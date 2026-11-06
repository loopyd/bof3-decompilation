#include "bof3/bof3.h"

/* @source 0x801EF350
 * @behavior Reads the scratchpad work-object pointer cell at 0x1F800044 once
 * and, only when bit 0x80 of the work object's byte at offset 0xB is set
 * (lbu / andi 0x80 / beqz, so the original tests the masked byte rather than a
 * value equality), increments the byte at offset 0x2 with a byte-width add (the
 * original loads it with lbu, adds 1 and stores it back with sb). The cell read
 * stays in $v1 across the guard, so both the tested byte and the incremented
 * byte come from one load of 0x1F800044. Leaf with no frame; nothing is
 * returned and no other state is read or written. Two exact siblings share the
 * skeleton of this guard-then-increment: the mask-free equality forms
 * incrementWorkByte1WhenWorkByteBIs80BmagicMagic11303_801EF3A0 and
 * incrementWorkByte2WhenWorkByteBZeroBmagicMagic15103_801F00B4, which keep the
 * pointer in $v1/$a0 the same way. This function's complete 56-byte body is
 * byte-identical to the unlifted overlay function at 0x801EF11C in
 * emi/bmagic/magic144/03, which is shape evidence only.
 * @status exact
 * @match 100.00
 * @residual none
 */
void incrementWorkByte2WhenWorkByteBFlag80SetBmagicMagic09203_801EF350(void) {
  u8 *work = SPAD_PTR_SLOT(u8, 0x44u);

  if (work[0xB] & 0x80) {
    work[2]++;
  }
}
