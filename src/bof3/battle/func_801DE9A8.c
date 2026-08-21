#include "bof3/battle/battle03_internal.h"

/* @behavior submits one local battler template selected by byte `0x79` through the
 * common script/event helper rooted at `0x801490d8`.
 * @source 0x801DE9A8
 * @status exact
 * @match 100.00
 * @residual none
 */
void func_801DE9A8(u32 arg0) {
  func_801501E4(D_801490D8,
                (void*)&D_80144968[D_80181B10[
                    D_80145E90[arg0 & 0xffu].unk_79]],
                5u);
}
