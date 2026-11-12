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

  func_801D9534(20, 18, 280, 19, 0);
  mask = 0;
  if (D_80145E93 >= 3) {
    mode = D_1F800044;
    mask = (u8)(1u << mode[6]);
  }
  drawLabeledStatusCellsWorld00Area03004_801D7C10(88, 40, mask);
  step = D_1F800044;
  step[9]--;
  panel = D_1F800044;
  drawEntryPanelStepStripAndClaimEntryWorld00Area03004_801D72E4(
      39, (s16)(64 + panel[9] * 60));
  state = D_1F800044;
  if (state[9] == 0) {
    index = (u8)func_8019601c();
    D_80143FC8[index].flags_00 = 1;
    D_80143FC8[index].byte_05 = 26;
    D_80143FC8[index].unk_01 = 16;
    values = D_1F800044;
    D_80143FC8[index].unk_02 = FIELD_REF(u16, values, 0x3E);
    next = D_1F800044;
    next[1]++;
  }
}
