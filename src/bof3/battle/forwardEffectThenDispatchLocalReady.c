#include "bof3/battle/battle03_internal.h"

/* @source 0x801E1C78
 * @behavior forwards the selected local effect id to the shared EXE-side
 * helper and then runs the local readiness helper selected by the current
 * local work word +0x124: bit 0x10 takes localReadyOrHelper1, otherwise bit
 * 0x20 takes localReadyOrHelper2.
 * @status exact
 * @match 100.00
 * @residual none
 */
void forwardEffectThenDispatchLocalReady(void) {
  u32 work_flags;

  forwardSelectedEffectId();
  work_flags = BATTLE_LOCAL_WORD_124(BATTLE_LOCAL_WORK_PTR);
  if ((work_flags & 0x10u) != 0u) {
    localReadyOrHelper1();
  } else if ((work_flags & 0x20u) != 0u) {
    localReadyOrHelper2();
  }
}
