#include "bof3/battle/battle15_internal.h"

/* @source 0x800A8D3C
 * @behavior Invokes the shared readiness helper at 0x801DEE4C, then invokes the
 * shared helper at 0x801E5988 while byte 0x01 of the local battle-work record
 * pointed to by 0x801EB4E0 equals 5.
 * @status exact
 * @match 100.00
 * @residual none
 */
void pollReadyThenInvokeHelperWhenWorkByte1Is5(void) {
  func_801DEE4C();
  if (D_801EB4E0->unk_01 == 5) {
    func_801E5988();
  }
}
