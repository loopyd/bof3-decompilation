#include "bof3/world/area01613_internal.h"

/* @source 0x801F3750
 * @behavior Retreats the scratch height at `0x30` by `0x0A` and increments local
 * state `0x03` while the signed height stays below `0xC9`; then arms state `0x03`
 * at `3` when the shared mode byte and the second scratch byte are both set, or
 * when the shared mode byte at `0x80143BB0` is `2`, or when the shared mask bit
 * `0x100` is set; finally calls the `0x801f40c4` local emitter with the scratch
 * height.
 * @status exact
 * @match 100.00
 * @residual none
 */
void func_801F3750(void) {
  World00Area016Scratch* scratch;

  scratch = D_1F800044;
  scratch->field_30 -= 0xA;
  if (scratch->field_30 < 0xC9) {
    scratch->state_03++;
  }

  if ((D_801454F2 != 0 && D_1F800044->unk_0b != 0) || D_80143BB0 == 2 ||
      (D_80146258 & 0x100u) != 0) {
    D_1F800044->state_03 = 3;
  }

  {
    World00Area016Scratch* scratch2;

    scratch2 = D_1F800044;
    func_801F40C4(0x5C, scratch2->field_30);
  }
}
