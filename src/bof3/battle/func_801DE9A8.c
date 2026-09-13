#include "bof3/battle/battle03_internal.h"

/* @behavior copies the selected local battler's name into text substitution slot
 * 0, taking at most five source bytes and appending a NUL terminator.
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
