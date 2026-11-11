#include "bof3/world/area03004_internal.h"

/* @source 0x801DC248
 * @behavior AREA030 panel step: increments slide byte 9 of the work record
 * published at the scratchpad cursor 0x1F800044, submits the panel pair at
 * (-80 * slide, 78), configures the sprite at 128 + 48 * slide with CLUT 91
 * and byte 6 of the active 0x98-byte work record D_80146888[D_801E3208],
 * draws that record byte together with the record byte at +0x90 through
 * func_801E1004, draws the icon row at (176, 156 + 30 * slide) and, while the
 * dispatch byte 0xB reads 1 or 2, sprite 8 at 148 + 30 * slide with image 68
 * (dispatch 1) or 66 (dispatch 2) through func_801E162C. Once the slide count
 * reaches 4 it clears byte 9 and selects the next work state: dispatch 1 with a
 * clear flag byte 6 selects state 11, every other case selects state 1, and
 * both paths clear the dispatch byte, where the second also advances the
 * front-end step counter D_8014403D. Both paths then start the front-end
 * selection effect and queue sound cue 0x203. The step ends with
 * func_8014D978.
 * @status exact
 * @match 100.00
 * @residual none
 */
void advanceEntrySlideAndSelectStateWorld00Area03004_801DC248(void) {
  u8* work;
  u8* state;
  u8* step_byte;

  D_1F800044[9]++;
  submitPanelPair((s16)(D_1F800044[9] * -80), 78);
  configureSpriteClut((s16)(128 + D_1F800044[9] * 48), 91,
                      D_80146888[D_801E3208].unk_02[4]);
  func_801E1004(D_80146888[D_801E3208].unk_02[4],
                D_80146888[D_801E3208].unk_60[0x30], D_1F800044[9]);
  func_801E1320(176, (s16)(D_1F800044[9] * 30 + 156));
  work = D_1F800044;
  if (work[0xB] == 1) {
    func_801E162C(8, work[9] * 30 + 148, 68);
  } else if (work[0xB] == 2) {
    func_801E162C(8, work[9] * 30 + 148, 66);
  }
  state = D_1F800044;
  if (state[9] == 4) {
    state[9] = 0;
    state = D_1F800044;
    if (state[0xB] == 1 && state[6] == 0) {
      state[2] = 0x0B;
      D_1F800044[3] = 0;
    } else {
      work = D_1F800044;
      work[2] = 1;
      D_1F800044[3] = 0;
      step_byte = &D_8014403D;
      *step_byte = *step_byte + 1;
    }
    func_8015D404(0, 1);
    func_80161C20(0x28, 0x64, 8);
    func_8015DF18(0x203);
  }
  func_8014D978();
}
