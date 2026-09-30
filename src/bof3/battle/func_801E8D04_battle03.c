#include "bof3/battle/battle03_internal.h"

/* @source 0x801E8D04
 * @behavior Clears work flag 0x40 and advances the work countdown word +0x40 by
 * 0x2000; when that word has already reached 0x10000 it instead clears work byte
 * +0x48, masks flag 0x2000 out of the 0x140-stride flag record selected by work
 * byte +0x05, masks 0xFFFCFFF9 into that record's second flag word, and clears
 * the queued-slot bytes.
 * @status exact
 * @match 100.00
 * @residual none
 */
void func_801E8D04(void) {
  Battle03LocalWork* work;

  D_801EB4E0->flags_00 &= 0xBFu;
  work = D_801EB4E0;
  if (work->unk_40 != 0x10000) {
    work->unk_40 += 0x2000;
    return;
  }
  work->unk_48 = 0;
  /* The mask block indexes through the published work pointer rather than the
   * `work` local: the pointer is loaded once into $a1 and the selection byte is
   * re-read by each statement. */
  D_80145FB4[D_801EB4E0->unk_05].flags_00 &= ~0x2000u;
  D_80145FB8[D_801EB4E0->unk_05].flags_00 &= 0xFFFCFFF9u;
  clearQueuedSlotBytes();
}
