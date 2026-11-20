#include "bof3/world/area03004_internal.h"

/* @source 0x801D4984
 * @behavior decrements work byte 9, draws the entry panels at offsets derived
 * from that byte, and uses work halfword 0x3E to select an available entry or
 * the 0xFF sentinel. Draws the fixed header and mode indicators, then advances
 * state byte 1 when the sliding byte reaches zero.
 * @status exact
 * @match 100.00
 * @residual none
 */
void decrementEntryPanelSlideAndAdvanceStateWorld00Area03004_801D4984(void) {
  u8 unused_stack[8];
  u8* step;
  u8* pair;
  u8* entry;
  u8* values;
  u8* icons;
  u8* texture;
  u8* mode;
  u8* state;
  s16 index;
  s32 value_index;
  u32 mask;

  step = D_1F800044;
  step[9]--;
  pair = D_1F800044;
  submitPanelPair((s16)(-(pair[9] * 80)), 78);
  entry = D_1F800044;
  index = FIELD_REF(s16, entry, 0x3E);
  if (D_80144FE4[index] != 0) {
    configureSpriteClut((s16)(128 + entry[9] * 48), 91, (u8)index);
    values = D_1F800044;
    value_index = FIELD_REF(s16, values, 0x3E);
    drawSlidingEntryLabelAndAmountSummaryWorld00Area03004((u8)value_index, D_80144FE4[value_index], values[9]);
  } else {
    configureSpriteClut((s16)(128 + entry[9] * 48), 91, 255);
    values = D_1F800044;
    drawSlidingEntryLabelAndAmountSummaryWorld00Area03004(255, 0, values[9]);
  }
  icons = D_1F800044;
  drawSlotValueSummaryAndThresholdIconsWorld00Area03004(176, (s16)(156 + icons[9] * 30));
  texture = D_1F800044;
  drawSpriteAndOptionalIndexedRunWorld00Area03004(8, 148 + texture[9] * 30, 0xFFFF);
  WORLD00_AREA030_DRAW_HEADER_AND_STATUS(mode, mask);
  state = D_1F800044;
  if (state[9] == 0) {
    state[1]++;
  }
}
