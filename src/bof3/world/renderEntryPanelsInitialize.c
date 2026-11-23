#include "bof3/world/area03004_internal.h"

/* @source 0x801D52D4
 * @behavior draws the fixed AREA030 panel and masked entry panels, clears work
 * bytes 8 and 10 and words 0x3C, 0x14 and 0x20, sets byte 9 to four, then
 * advances state byte 1. The entry mask is zero below shared mode three.
 * @status exact
 * @match 100.00
 * @residual none
 */
void renderEntryPanelsInitialize(void) {
  u8* cursor;
  u8* cleared;
  u8* initialized;
  u8* state;
  u32 mask;

  WORLD00_AREA030_DRAW_HEADER_AND_STATUS(cursor, mask);
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
