#include "internal.h"

/* @behavior waits on the second readiness helper, advances local substate byte `3`
 * when ready, and conditionally emits the followup effect path.
 * @source 0x801E019C
 */
void waitReadyAdvanceSubstate3(void) {
  if (func_8014DAEC() != 0u) {
    BATTLE_LOCAL_SCRATCH_PTR->unk_03 += 1u;
    if ((BATTLE_LOCAL_FLAGS_80(BATTLE_LOCAL_WORK_PTR) & 4u) == 0u) {
      func_801DEFE4();
    }
  }
}
