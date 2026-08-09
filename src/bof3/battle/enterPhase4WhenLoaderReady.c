#include "bof3/battle/battle03_internal.h"

/* @behavior advances the local phase once the EXE-side EMI loader is ready.
 * @source 0x801D716C
 * @status exact
 * @match 100.00
 * @residual none; live audit is instruction- and byte-exact.
 */
void enterPhase4WhenLoaderReady(void) {
  if (func_80162D00() != 0) {
    *D_801462E1 = 4;
    BATTLE_GLOBAL_BYTE_62E2 = 0;
  }
}
