#include "bof3/world/area01613_internal.h"

/* @source 0x801F3928
 * @behavior Advances the scratch height at `0x30` by `0x0A`, clears local state `0x03` once the reset height reaches `0xF0`, then arms state `0x03` when the shared mode byte is zero, the second shared mode byte is not `2` and the shared mask bit `0x100` is unset; finally calls the `0x801f40c4` local emitter with the scratch height.
 * @status exact
 * @match 100.00
 * @residual none
 */
void func_801F3928(void) {
  World00Area016Scratch* scratch;

  scratch = D_1F800044;
  scratch->field_30 += 0xA;
  if (scratch->field_30 >= 0xF0) {
    scratch->state_03 = 0;
  }

  if (D_801454F2 == 0 && D_80143BB0 != 2 && (D_80146258 & 0x100u) == 0) {
    D_1F800044->state_03 = 1;
  }

  {
    World00Area016Scratch* scratch2;

    scratch2 = D_1F800044;
    func_801F40C4(0x5C, scratch2->field_30);
  }
}
