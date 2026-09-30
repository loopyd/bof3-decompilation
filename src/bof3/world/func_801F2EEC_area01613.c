#include "bof3/world/area01613_internal.h"

/* @source 0x801F2EEC
 * @behavior Projects the shared scroll offsets at 0x80145E9C and 0x80145EA0,
 * scaled by the shared byte at 0x80145E99, onto the scratch words at offsets
 * `0x0C` and `0x10`, then selects the scratch byte at offset `0x0B` from the
 * probe helper's code (`0xA1`, `0xA0`, `0xAE`, otherwise `4` when the shared
 * flag word at 0x8014625A has bit `0x1000` set, otherwise `0`), and finally
 * dispatches the scratch mode byte at offset `0x01` through the local handler
 * table at 0x801F5100.
 * @status exact
 * @match 100.00
 * @residual none
 */
void NO_SIBLING_CALLS func_801F2EEC(void) {
  World00Area016Scratch* scratch;
  World00Area016Scratch* state;
  s32 scale;
  s16 x;
  s16 y;
  u8 code;

  scale = D_80145E99;
  x = (D_80145EC4 + D_80145E9C * scale) >> 16;
  y = (D_80145EC8 + D_80145EA0 * scale) >> 16;

  scratch = D_1F800044;
  scratch->unk_0c = x;
  scratch->unk_10 = y;

  code = (u8)func_80166CB0(x, y);
  if (code == 0xA1) {
    state = D_1F800044;
    state->unk_0b = 1;
  } else {
    code = (u8)func_80166CB0(x, y);
    if (code == 0xA0) {
      state = D_1F800044;
      state->unk_0b = 2;
    } else {
      code = (u8)func_80166CB0(x, y);
      if (code == 0xAE) {
        state = D_1F800044;
        state->unk_0b = 3;
      } else if (D_8014625A & 0x1000) {
        state = D_1F800044;
        state->unk_0b = 4;
      } else {
        D_1F800044->unk_0b = 0;
      }
    }
  }

  D_801F5100[WORLD00_AREA016_SCRATCH_PTR->mode]();
}
