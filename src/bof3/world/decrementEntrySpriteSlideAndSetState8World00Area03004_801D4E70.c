#include "bof3/world/area03004_internal.h"

void drawEntryPanelCounterRowWorld00Area03004(void);

/* @source 0x801D4E70
 * @behavior decrements work byte 9, draws the entry panels and slides the
 * selected entry sprite right or left according to work byte 8. Uses the
 * 0xFF sentinel for an unavailable entry, draws the fixed header and mode
 * indicators, and sets state byte 1 to 8 when the sliding byte reaches zero.
 * @status exact
 * @match 100.00
 * @residual none
 */
void decrementEntrySpriteSlideAndSetState8World00Area03004_801D4E70(void) {
  u8* step;
  u8* entry;
  u8* sprite;
  u8* mode;
  u8* state;
  s16 index;
  u32 amount;
  u32 mask;

  step = D_1F800044;
  index = (s16)(step[9] - 1);
  step[9] = (u8)index;
  submitPanelPair(0, 78);
  entry = D_1F800044;
  index = FIELD_REF(s16, entry, 0x3E);
  amount = D_80144FE4[index];
  if (amount != 0) {
    drawSlidingEntryLabelAndAmountSummaryWorld00Area03004((u8)index, amount, 0);
  } else {
    drawSlidingEntryLabelAndAmountSummaryWorld00Area03004(255, 0, 0);
  }
  sprite = D_1F800044;
  if (sprite[8] == 0) {
    s16 sprite_index = FIELD_REF(s16, sprite, 0x3E);

    if (D_80144FE4[sprite_index] != 0) {
      configureSpriteClut((s16)(128 + sprite[9] * 48), 91, (u8)sprite_index);
    } else {
      configureSpriteClut((s16)(128 + sprite[9] * 48), 91, 255);
    }
  } else {
    s16 sprite_index = FIELD_REF(s16, sprite, 0x3E);

    if (D_80144FE4[sprite_index] != 0) {
      configureSpriteClut((s16)(128 - sprite[9] * 48), 91, (u8)sprite_index);
    } else {
      configureSpriteClut((s16)(128 - sprite[9] * 48), 91, 255);
    }
  }
  drawSlotValueSummaryAndThresholdIconsWorld00Area03004(176, 156);
  drawSpriteAndOptionalIndexedRunWorld00Area03004(8, 148, 0xFFFF);
  drawEntryPanelCounterRowWorld00Area03004();
  WORLD00_AREA030_DRAW_HEADER_AND_STATUS(mode, mask);
  state = D_1F800044;
  if (state[9] == 0) {
    state[1] = 8;
  }
}
