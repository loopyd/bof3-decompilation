#include "bof3/world/area03004_internal.h"

void drawEntryPanelStepStripAndClaimEntryWorld00Area03004_801D72E4(s32 x,
                                                                  s32 y);

/* @source 0x801D5574
 * @behavior increments work byte 9 and draws the header, entry panels and
 * scrolling panel at offsets derived from that byte. At step four, clears
 * state byte 1 if byte 0xB is zero; otherwise selects state 2 for mode zero
 * or state 6 for mode one, leaving other modes unchanged.
 * @status exact
 * @match 100.00
 * @residual none
 */
void advanceEntryPanelSlideAndSelectStateWorld00Area03004_801D5574(void) {
  u8* step;
  u8* header;
  u8* entries;
  u8* mode;
  u8* panel;
  u8* state;
  u32 mask;

  step = D_1F800044;
  step[9]++;
  header = D_1F800044;
  if (header[0xB] == 0) {
    func_801D9534(20, (18 - header[9] * 16) & 0xFFFE, 280, 19, 0);
    entries = D_1F800044;
    drawLabeledStatusCellsWorld00Area03004_801D7C10((u16)(88 + entries[9] * 45), 40, 0);
  } else {
    func_801D9534(20, 18, 280, 19, 0);
    mask = 0;
    if (D_80145E93 >= 3) {
      mode = D_1F800044;
      mask = (u8)(1u << mode[6]);
    }
    drawLabeledStatusCellsWorld00Area03004_801D7C10(88, 40, mask);
  }
  panel = D_1F800044;
  drawEntryPanelStepStripAndClaimEntryWorld00Area03004_801D72E4(
      39, (s16)(64 + panel[9] * 60));
  state = D_1F800044;
  if (state[9] == 4) {
    if (state[0xB] == 0) {
      state[1] = 0;
    } else if (state[6] == 0) {
      state[1] = 2;
    } else if (state[6] == 1) {
      state[1] = 6;
    }
  }
}
