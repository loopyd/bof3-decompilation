#include "bof3/world/area03004_internal.h"

/* @source 0x801D5048
 * @behavior slides the entry panels and selected sprite using work byte 9,
 * then increments that byte. Draws the header and mode indicators at sliding
 * positions, using the unavailable-entry sentinel when needed. At count four,
 * selects state byte 1 from work bytes 0x0B and 6.
 * @status exact
 * @match 100.00
 * @residual none
 */
void advanceEntryPanelAndSpriteSlideAndSelectStateWorld00Area03004_801D5048(void) {
  u8* panel;
  u8* sliding;
  u8* pair;
  u8* value;
  u8* image;
  u8* mode;
  u8* step;
  u8* entry;
  u8* sprite;
  u8* state;
  s16 index;
  u32 mask;

  panel = D_1F800044;
  if (panel[0x0B] == 0) {
    renderFilledInsetBorder(20, (18 - panel[9] * 16) & 0xFFFE, 280, 19, 0);
    sliding = D_1F800044;
    renderLabeledStatusCells(88 + sliding[9] * 45, 40, 0);
  } else {
    renderFilledInsetBorder(20, 18, 280, 19, 0);
    if (D_80145E93 >= 3u) {
      mode = D_1F800044;
      mask = (u8)(1u << mode[6]);
    } else {
      mask = 0u;
    }
    renderLabeledStatusCells(88, 40, mask);
  }
  step = D_1F800044;
  step[9]++;
  pair = D_1F800044;
  submitPanelPair((s16)(-(pair[9] * 80)), 78);
  entry = D_1F800044;
  index = FIELD_REF(s16, entry, 0x3E);
  if (D_80144FE4[index] != 0) {
    configureSpriteClut((s16)(128 + entry[9] * 48), 91, (u8)index);
    sprite = D_1F800044;
    {
      s16 sprite_index = FIELD_REF(s16, sprite, 0x3E);

      renderSlidingAmountSummary((u8)sprite_index, D_80144FE4[sprite_index], sprite[9]);
    }
  } else {
    configureSpriteClut((s16)(128 + entry[9] * 48), 91, 255);
    sprite = D_1F800044;
    renderSlidingAmountSummary(255, 0, sprite[9]);
  }
  value = D_1F800044;
  renderSummaryThresholdIcons(176, (s16)(156 + value[9] * 30));
  image = D_1F800044;
  renderSpriteOptionalIndexed(8, 148 + image[9] * 30, 0xFFFF);
  state = D_1F800044;
  if (state[9] == 4) {
    if (state[0x0B] == 0) {
      state[1] = 0;
    } else if (state[6] == 0) {
      state[1] = 2;
    } else if (state[6] == 2) {
      state[1]++;
    }
  }
}
