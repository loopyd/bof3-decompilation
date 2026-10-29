#include "bof3/bof3.h"

extern u16 D_80145AA4;

/* @source 0x801F3E50
 * @behavior Overlay state setter: when the shared pad word D_80145AA4 reads
 * exactly 0x3000, 0x6000 or 0x2000, it loads the work-object pointer published
 * at the scratchpad cell 0x1F800044 and stores 1 into that object's byte at
 * offset 0x01. The three tests are whole-word equality against the loaded
 * halfword - the original has no mask instruction - and the first constant is
 * materialized before the first branch while the second and third fill the
 * following branch delay slots. The stored byte is the state byte the exact
 * siblings resetScratchStateToMode1World03Area13413_801F35A4 (mode 1),
 * countdownScratchStateToMode15World03Area13413_801F3B7C (mode 0x0F) and
 * countdownScratchStateToMode17World03Area13413_801F32E4 (mode 0x11) arm
 * through the same cell, so this handler arms mode 1 of that family when the
 * pad word matches. It takes no arguments, makes no call, keeps no register
 * and reads no other state, so its 0x3C bytes carry no frame. The same three
 * values gate a path of the neighbouring func_801F3F78 in this overlay at
 * 0x801F4060.
 * @status exact
 * @match 100.00
 * @residual none
 */
void setScratchStateToMode1WhenPadMatchesWorld03Area13413_801F3E50(void) {
  u8** slots;

  if (D_80145AA4 == 0x3000 || D_80145AA4 == 0x6000 || D_80145AA4 == 0x2000) {
    slots = SPAD_PTR_TABLE(u8);
    slots[0x11][0x01] = 1;
  }
}
