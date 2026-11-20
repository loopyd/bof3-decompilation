#include "bof3/world/area03004_internal.h"

void drawEntryPanelCounterRowWorld00Area03004(void);

/* @source 0x801D4B60
 * @behavior draws the selected entry panels at fixed coordinates, using work
 * halfword 0x3E and its entry-amount byte when available or the 0xFF/0xFFFF
 * sentinels otherwise. Draws the entry number, header and mode indicators.
 * @status exact
 * @match 100.00
 * @residual none
 */
void drawSelectedEntryPanelsWorld00Area03004_801D4B60(void) {
  u8* entry;
  u8* sprite;
  u8* texture;
  u8* mode;
  s16 index;
  u32 amount;
  u32 mask;

  submitPanelPair(0, 78);
  entry = D_1F800044;
  index = FIELD_REF(s16, entry, 0x3E);
  amount = D_80144FE4[index];
  if (amount != 0) {
    drawSlidingEntryLabelAndAmountSummaryWorld00Area03004((u8)index, amount, 0);
    sprite = D_1F800044;
    configureSpriteClut(128, 91, sprite[0x3E]);
    texture = D_1F800044;
    drawSpriteAndOptionalIndexedRunWorld00Area03004(8, 148, FIELD_REF(u16, texture, 0x3E));
  } else {
    drawSlidingEntryLabelAndAmountSummaryWorld00Area03004(255, 0, 0);
    configureSpriteClut(128, 91, 255);
    drawSpriteAndOptionalIndexedRunWorld00Area03004(8, 148, 0xFFFF);
  }
  drawSlotValueSummaryAndThresholdIconsWorld00Area03004(176, 156);
  drawEntryPanelCounterRowWorld00Area03004();
  WORLD00_AREA030_DRAW_HEADER_AND_STATUS(mode, mask);
}
