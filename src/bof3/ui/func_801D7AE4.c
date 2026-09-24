#include "bof3/ui/shop00_internal.h"

/* @source 0x801D7AE4
 * @behavior Shop phase step that runs before the field-list setup:
 *           calls the shop setup helper buildSlotList with 1, then the panel
 *           setup helper func_801D7E1C with the work-block index byte
 *           D_80148656[0], the phase-relative record offset
 *           (phaseTimer << 5) + 0x98, 0x3F, 0 and 0xFF. It then advances the
 *           frame timer phaseTimer by one and, when the advanced byte equals
 *           7, clears phaseTimer and the sub-step byte D_80148652, arms
 *           D_80148650 with 1 and clears the phase byte D_80148651 (sibling
 *           shapes: tickPhaseTimer and func_801DEE20).
 * @status exact
 * @match 100.00
 * @residual none
 */
void func_801D7AE4(void) {
  u8 next;

  buildSlotList(1);
  func_801D7E1C(D_80148656[0], (phaseTimer << 5) + 0x98, 0x3F, 0, 0xFF);
  next = phaseTimer + 1;
  phaseTimer = next;
  if (next == 7) {
    phaseTimer = 0;
    D_80148650 = 1;
    D_80148651 = 0;
    D_80148652 = 0;
  }
}
