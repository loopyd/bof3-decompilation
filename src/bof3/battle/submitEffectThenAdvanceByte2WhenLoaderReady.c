#include "bof3/battle/battle03_internal.h"

/* @source 0x801E1D1C
 * @behavior Once the EXE-side EMI loader reports ready (func_80162D00),
 * submits effect id scratch byte +0x08 plus 0x24 to func_8014D8D4, then runs
 * the second local readiness helper localReadyOrHelper2 and advances the
 * scratch work record byte +0x02.
 * @status exact
 * @match 100.00
 * @residual none
 */
void submitEffectThenAdvanceByte2WhenLoaderReady(void) {
  if (func_80162D00() != 0) {
    func_8014D8D4(D_1F800044->unk_08 + 0x24u);
    localReadyOrHelper2();
    D_1F800044->unk_02++;
  }
}
