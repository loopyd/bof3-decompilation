#include "bof3/world/area03004_internal.h"

void drawEntryPanelStepStripAndClaimEntryWorld00Area03004_801D72E4(s32 x,
                                                                  s32 y);

/* @source 0x801D5398
 * @behavior draws the header and masked entry panels, decrements work byte 9,
 * and draws the sliding panel. At zero, initializes the returned entry record
 * with bytes 0 = 1, 5 = 26, 1 = 16 and 2 = the low byte of work halfword 0x3E,
 * then advances state byte 1. The returned record index is not sentinel-checked.
 * @status exact
 * @match 100.00
 * @residual none
 */
void decrementEntryPanelSlideInitializeRecordAndAdvanceStateWorld00Area03004_801D5398(void) {
  u8* mode;
  u8* step;
  u8* panel;
  u8* state;
  u8* values;
  u8* next;
  u32 mask;
  u32 index;

  WORLD00_AREA030_DRAW_HEADER_AND_STATUS(mode, mask);
  step = D_1F800044;
  step[9]--;
  panel = D_1F800044;
  drawEntryPanelStepStripAndClaimEntryWorld00Area03004_801D72E4(
      39, (s16)(64 + panel[9] * 60));
  state = D_1F800044;
  if (state[9] == 0) {
    index = (u8)findFreeRecord();
    WORLD00_AREA030_ARM_PANEL_ENTRY(index);
    values = D_1F800044;
    D_80143FC8[index].unk_02 = FIELD_REF(u16, values, 0x3E);
    next = D_1F800044;
    next[1]++;
  }
}
