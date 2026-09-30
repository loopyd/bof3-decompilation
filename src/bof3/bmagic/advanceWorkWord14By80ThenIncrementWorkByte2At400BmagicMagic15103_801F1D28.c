#include "bof3/bof3.h"

/* @source 0x801F1D28
 * @behavior Reads the scratchpad work-object pointer cell at 0x1F800044 once,
 * loads the work object's word at offset 0x14, advances it by 0x80 and stores
 * the advanced word back unconditionally (the original writes it with sw in
 * the taken/not-taken branch delay slot). When that new word equals 0x400 the
 * work object's byte at offset 0x2 is incremented with a byte-width add (lbu /
 * addiu 1 / sb); nothing is returned and no other state is read or written.
 * @status exact
 * @match 100.00
 * @residual none
 */
void advanceWorkWord14By80ThenIncrementWorkByte2At400BmagicMagic15103_801F1D28(void) {
  u8 *work = SPAD_PTR_SLOT(u8, 0x44u);
  s32 value;

  value = *(s32 *)(work + 0x14) + 0x80;
  *(s32 *)(work + 0x14) = value;
  if (value == 0x400) {
    work[2]++;
  }
}
