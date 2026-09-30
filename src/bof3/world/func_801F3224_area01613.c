#include "bof3/world/area01613_internal.h"

/* @source 0x801F3224
 * @behavior Seeds the shared scratch defaults, decrements the scratch counter
 * byte at offset `0x09`, advances the scratch word at offset `0x40` by `0x2000`,
 * then, once that counter reaches zero, clears the scratch byte at offset `0x48`
 * and sets the mode byte at offset `0x01` to `3` before calling the shared
 * `0x8014D4E0` helper.
 * @status exact
 * @match 100.00
 * @residual none
 */
void func_801F3224(void) {
  World00Area016Scratch* scratch;
  World00Area016Scratch* state;

  seedScratchDefaults();
  scratch = D_1F800044;
  scratch->unk_04[5]--;
  scratch->unk_40 += 0x2000;

  state = D_1F800044;
  if (state->unk_04[5] == 0) {
    state->unk_48 = 0;
    D_1F800044->mode = 3;
  }

  func_8014D4E0();
}
