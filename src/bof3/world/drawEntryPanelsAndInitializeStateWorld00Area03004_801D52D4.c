#include "bof3/world/area03004_internal.h"

/* @source 0x801D52D4
 * @behavior draws the fixed AREA030 panel and masked entry panels, clears work
 * bytes 8 and 10 and words 0x3C, 0x14 and 0x20, sets byte 9 to four, then
 * advances state byte 1. The entry mask is zero below shared mode three.
 * @status exact
 * @match 100.00
 * @residual none
 */
void drawEntryPanelsAndInitializeStateWorld00Area03004_801D52D4(void) {
  u8* cursor;
  u8* cleared;
  u8* initialized;
  u8* state;
  u32 mask;

  func_801D9534(20, 18, 280, 19, 0);
  mask = 0u;
  if (D_80145E93 >= 3u) {
    cursor = D_1F800044;
    mask = (u8)(1u << cursor[6]);
  }
  drawLabeledStatusCellsWorld00Area03004_801D7C10(88, 40, mask);
  cursor = D_1F800044;
  cursor[8] = 0;
  cleared = D_1F800044;
  cleared[10] = 0;
  initialized = D_1F800044;
  *(u32*)(cleared + 0x3C) = 0;
  *(u32*)(cleared + 0x14) = 0;
  *(u32*)(cleared + 0x20) = 0;
  initialized[9] = 4;
  state = D_1F800044;
  state[1]++;
}
