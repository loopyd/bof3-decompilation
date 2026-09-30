#include "bof3/battle/battle03_internal.h"

/* @source 0x801E7778
 * @behavior Clears work flag 0x40 and advances the work countdown word +0x40 by
 * 0x2000; when that word has already reached 0x10000 it instead clears work byte
 * +0x48, clears flag 0x2000 in the flag record selected by work byte +0x05, and
 * clears the queued-slot bytes.
 * @status exact
 * @match 100.00
 * @residual none
 */
void func_801E7778(void) {
  Battle03LocalWork* work;
  u8 selection;

  D_801EB4E0->flags_00 &= 0xBFu;
  work = D_801EB4E0;
  if (work->unk_40 != 0x10000) {
    work->unk_40 += 0x2000;
    return;
  }
  work->unk_48 = 0;
  selection = D_801EB4E0->unk_05;
  D_80145FB4[selection].flags_00 &= ~0x2000u;
  clearQueuedSlotBytes();
}
