#include "bof3/world/area03004_internal.h"

void drawEntryPanelStepStripAndClaimEntryWorld00Area03004_801D72E4(s32 x,
                                                                  s32 y);

/* @source 0x801D54F8
 * @behavior Draws the header, status cells and fixed entry panel. Actor state
 * byte 3 enables a status mask selected by work byte 6 through the shared
 * drawing macro. The shift-count domain remains unverified.
 * @status exact
 * @match 100.00
 * @residual none
 */
void drawHeaderStatusAndFixedEntryPanelWorld00Area03004_801D54F8(void) {
  u8* cursor;
  u32 mask;

  WORLD00_AREA030_DRAW_HEADER_AND_STATUS(cursor, mask);
  drawEntryPanelStepStripAndClaimEntryWorld00Area03004_801D72E4(39, 64);
}
