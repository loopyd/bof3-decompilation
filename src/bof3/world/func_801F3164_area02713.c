#include "bof3/world/area02713_internal.h"

/* @behavior polls the shared loader on 0x80144E98 with request 0x17: when the
 * poll succeeds it sets the local scratch byte at offset 2 to 3 and runs
 * 0x801F33A8, otherwise it advances that byte and emits the two fixed world
 * markers through `emitMarkerPair`.
 * @source 0x801F3164
 * @status exact
 * @match 100.00
 * @residual none
 */
void func_801F3164(void) {
  if (func_8015B5D4((u32)D_80144E98, 0x17) != 0) {
    WORLD00_AREA027_SCRATCH_PTR[2] = 3;
    func_801F33A8();
  } else {
    WORLD00_AREA027_SCRATCH_PTR[2] += 1u;
    emitMarkerPair();
  }
}
