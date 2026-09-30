#include "bof3/world/area01613_internal.h"

/* @source 0x801F3828
 * @behavior Counts the scratch byte at offset `0x09` while scratch byte `0x0B` is
 * clear and sets `0x0B` once that count reaches `0x5A`; then arms local state
 * `0x03` when the shared mode byte and scratch byte `0x0B` are both set, or when
 * the shared mode byte at `0x80143BB0` is `2`, or when the shared mask bit
 * `0x100` is set; finally calls the `0x801F40C4` local emitter with the scratch
 * height.
 * @status exact
 * @match 100.00
 * @residual none
 */
void func_801F3828(void) {
  World00Area016Scratch* scratch;
  World00Area016Scratch* reload;

  scratch = D_1F800044;
  if (scratch->unk_0b == 0) {
    scratch->unk_04[5]++;
    reload = D_1F800044;
    if (reload->unk_04[5] >= 0x5A) {
      reload->unk_0b++;
    }
  }

  if ((D_801454F2 != 0 && D_1F800044->unk_0b != 0) || D_80143BB0 == 2 ||
      (D_80146258 & 0x100u) != 0) {
    reload = D_1F800044;
    reload->state_03++;
  }

  {
    World00Area016Scratch* scratch2;

    scratch2 = D_1F800044;
    func_801F40C4(0x5C, scratch2->field_30);
  }
}
