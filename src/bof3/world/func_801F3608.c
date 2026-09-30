#include "bof3/world/area01613_internal.h"

/* @source 0x801F3608
 * @behavior Retreats the scratch height at `0x2e` by `0x10` and clears local
 * state `0x02` once the signed height drops below `-0x2f`; then arms state
 * `0x02` when the shared mode byte is not `2`, and finally calls the
 * `0x801f3b00` local step with the scratch height.
 * @status exact
 * @match 100.00
 * @residual none
 */
void func_801F3608(void) {
  World00Area016Scratch* scratch;

  scratch = D_1F800044;
  scratch->field_2e -= 0x10;
  if (scratch->field_2e < -0x2F) {
    scratch->state_02 = 0;
  }

  if (D_801454F2 != 2) {
    D_1F800044->state_02 = 1;
  }

  {
    World00Area016Scratch* scratch2;

    scratch2 = D_1F800044;
    func_801F3B00(0x10, scratch2->field_2e);
  }
}
