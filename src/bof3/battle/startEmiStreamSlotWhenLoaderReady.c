#include "bof3/battle/battle03_internal.h"

/* @source 0x801D7108
 * @behavior Once the EXE-side EMI loader reports ready, raises the 0x20
 * transfer flag of the shared EMI loader state byte D_80146854, starts the EMI
 * stream slot selected by the shared u16 D_80143F00 plus 0x2AB through
 * func_80161FDC, and advances the battle loader state byte
 * BATTLE_GLOBAL_BYTE_62E2.
 * @status exact
 * @match 100.00
 * @residual none
 */
void startEmiStreamSlotWhenLoaderReady(void) {
  volatile u8* state;

  if (func_80162D00() != 0) {
    D_80146854 |= 0x20u;
    func_80161FDC(D_80143F00 + 0x2ABu);
    state = &BATTLE_GLOBAL_BYTE_62E2;
    *state += 1;
  }
}
