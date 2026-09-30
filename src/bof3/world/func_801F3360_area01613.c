#include "bof3/world/area01613_internal.h"

/* @source 0x801F3360
 * @behavior Seeds the shared scratch defaults, decrements the scratch counter
 * byte at offset `0x09` and retreats the scratch word at offset `0x40` by
 * `0x2000`, then, once that counter reaches zero, calls the shared `0x80196070`
 * helper when the shared mode byte at `0x80143BB0` is `5` and otherwise clears
 * the scratch byte at offset `0x48` and sets the mode byte at offset `0x01` to
 * `1`; while the counter is still non-zero it calls the shared `0x8014D4E0`
 * helper instead.
 * @status exact
 * @match 100.00
 * @residual none
 */
void func_801F3360(void) {
  World00Area016Scratch* scratch;
  World00Area016Scratch* state;

  seedScratchDefaults();
  scratch = D_1F800044;
  scratch->unk_04[5]--;
  scratch->unk_40 -= 0x2000;

  state = D_1F800044;
  if (state->unk_04[5] == 0) {
    if (D_80143BB0 == 5) {
      func_80196070();
    } else {
      state->unk_48 = 0;
      scratch = D_1F800044;
      scratch->mode = 1;
    }
  } else {
    func_8014D4E0();
  }
}
