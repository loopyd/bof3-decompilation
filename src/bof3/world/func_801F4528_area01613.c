#include "bof3/world/area01613_internal.h"

/* @source 0x801F4528
 * @behavior Reads the scratch pointer at `0x1F800044`; when the scratch byte at
 * offset `0x0B` is non-zero and bit `0x80` of the scratch byte at offset `0x00`
 * is set it calls the shared `0x80196070` helper and returns, otherwise it sets
 * that byte to `1` when both it and the bit are clear, then advances the scratch
 * words at offsets `0x34` and `0x38` by the words at offsets `0x0C` and `0x10`
 * before calling the shared `0x8014D978` and `0x8014D290` helpers in that order.
 * @status exact
 * @match 100.00
 * @residual none
 */
void func_801F4528(void) {
  World00Area016Scratch* scratch;
  World00Area016Scratch* state;

  scratch = D_1F800044;
  if (scratch->unk_0b != 0) {
    if ((scratch->unk_00 & 0x80) != 0) {
      func_80196070();
      return;
    }
  } else if ((scratch->unk_00 & 0x80) == 0) {
    scratch->unk_0b = 1;
  }

  state = D_1F800044;
  state->unk_34 += state->unk_0c;
  state->unk_38 += state->unk_10;
  func_8014D978();
  func_8014D290();
}
