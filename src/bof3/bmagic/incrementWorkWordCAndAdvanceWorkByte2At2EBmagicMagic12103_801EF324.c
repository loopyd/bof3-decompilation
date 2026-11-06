#include "bof3/bof3.h"

/* @source 0x801EF324
 * @behavior Reads the scratchpad work-object pointer cell at 0x1F800044 once,
 * loads the work object's word at offset 0xC, advances it by one and stores the
 * advanced word back unconditionally (the original writes it with sw in the
 * taken/not-taken branch delay slot). When that new word equals 0x2E the work
 * object's byte at offset 0x2 is incremented with a byte-width add (lbu /
 * addiu 1 / sb), advancing to the next byte-0x2 handler step; nothing is
 * returned and no other state is read or written.
 * @status exact
 * @match 100.00
 * @residual none
 */
void incrementWorkWordCAndAdvanceWorkByte2At2EBmagicMagic12103_801EF324(
    void) {
  u8 *work = SPAD_PTR_SLOT(u8, 0x44u);
  s32 value;

  value = *(s32 *)(work + 0xC) + 1;
  *(s32 *)(work + 0xC) = value;
  if (value == 0x2E) {
    work[2]++;
  }
}
