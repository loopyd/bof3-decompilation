#include "bof3/ui/sisyou00_internal.h"

/* @source 0x801D2614
 * @behavior when the EXE-side flag helper func_8015B5D4 reports the
 *           D_8014554F byte flag as set, stores mode index 5; otherwise, when
 *           masterIndex is 9, starts action 46 through func_80150224, writes
 *           the phase byte D_80143BB0 as 2 and stores mode index 6; every
 *           other master index stores mode index 1.
 * @status exact
 * @match 100.00
 * @residual none
 */
void func_801D2614(void) {
  s32 index;

  if (func_8015B5D4((u32)&D_8014554F, masterIndex) != 0) {
    index = 5;
  } else if (masterIndex == 9) {
    func_80150224(46);
    D_80143BB0 = 2;
    index = 6;
  } else {
    index = 1;
  }
  modeIndex = index;
}
