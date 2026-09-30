#include "bof3/world/area01613_internal.h"

/* @source 0x801F32A0
 * @behavior Seeds the shared scratch defaults and then, unless the shared state
 * word at `0x80143B90` is `1`, resets the area scratch record when the scratch
 * byte at offset `0x0B` differs from the byte at offset `0x07`, or the shared
 * mode byte at `0x80143BB0` is `5`, or scratch byte `0x0B` is `1` while the
 * scratch word at offset `0x18` differs from the shared context seed at
 * `0x80143F10`; the reset stores `2` to scratch offset `0x48`, `8` to offset
 * `0x09` and `4` to the mode byte at offset `0x01`, then the shared
 * `0x8014D4E0` helper runs.
 * @status exact
 * @match 100.00
 * @residual none
 */
void func_801F32A0(void) {
  World00Area016Scratch* scratch;

  seedScratchDefaults();
  if (D_80143B90 != 1) {
    scratch = D_1F800044;
    if (scratch->unk_0b != scratch->unk_04[3] || D_80143BB0 == 5 ||
        (scratch->unk_0b == 1 && scratch->unk_18 != D_80143F10)) {
      D_1F800044->unk_48 = 2;
      D_1F800044->unk_04[5] = 8;
      D_1F800044->mode = 4;
    }
  }

  func_8014D4E0();
}
