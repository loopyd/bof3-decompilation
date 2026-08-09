#include "bof3/battle/battle03_internal.h"

/* @behavior selects one of two local-work-driven effect ids based on flag `0x800`
 * and forwards the result to the shared EXE-side helper.
 * @source 0x801DF34C
 * @status exact
 * @match 100.00
 * @residual none; live audit is instruction- and byte-exact.
 */
void forwardSelectedEffectId(void) {
  if ((BATTLE_LOCAL_FLAGS_80(BATTLE_LOCAL_WORK_PTR) & 0x0800u) != 0u) {
    func_8014D8D4(BATTLE_LOCAL_SCRATCH_PTR->unk_08 + 0x34u);
  } else {
    func_8014D8D4(BATTLE_LOCAL_SCRATCH_PTR->unk_08 + 0x10u);
  }
}
