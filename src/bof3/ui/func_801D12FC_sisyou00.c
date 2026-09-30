#include "bof3/ui/sisyou00_internal.h"

/* @source 0x801D12FC
 * @behavior advances the shared step byte at 0x801D4289, draws every one of
 *           the D_80146254 0x140-byte-stride records at 0x80145FCC through
 *           func_801D177C and func_801D1444, emits the shared panel strip
 *           through func_801D1E54, then advances the handler index once the
 *           step byte reaches 4.
 * @status exact
 * @match 100.00
 * @residual none
 */
void func_801D12FC(void) {
  s32 i;

  D_801D4289 = D_801D4289 + 1;

  for (i = 0; i < D_80146254; i++) {
    func_801D177C(0x11 - D_801D4289 * 36, 0x3E + i * 0x36,
                  D_80145FCC[i * 0x140], i);
    func_801D1444(D_801D4289 * 40 + 0x89, 0x3E + i * 0x36,
                  D_80145FCC[i * 0x140]);
  }

  func_801D1E54(0x14, (0x10 - D_801D4289 * 10) & 0xFFFE, 0x118, 0x13,
                D_80144952);

  if (D_801D4289 == 4) {
    D_801D4286 = D_801D4286 + 1;
  }
}
