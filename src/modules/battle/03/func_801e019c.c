#include "internal.h"

/* @behavior waits on the second readiness helper, advances local substate byte `3`
 * when ready, and conditionally emits the followup effect path.
 * @source 0x801e019c FUN_801e019c
 */
void func_801e019c(void) {
  if (func_8014daec() != 0u) {
    BATTLE_LOCAL_SCRATCH_PTR->unk_03 += 1u;
    if ((BATTLE_LOCAL_FLAGS_80(BATTLE_LOCAL_WORK_PTR) & 4u) == 0u) {
      func_801defe4();
    }
  }
}
