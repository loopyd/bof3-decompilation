#include "bof3/world/area03004_internal.h"

/* @behavior Submit the fixed panel rectangle, draw the equipment guide and lure-list
 * column, and draw the header status cells. Their mask is zero below shared
 * state 3 and otherwise selects the bit named by work byte 6.
 * @source 0x801D45F8
 * @status exact
 * @match 100.00
 * @residual none
 */
void renderHeaderLureList(void) {
  u8* statusWork;
  u32 statusMask;

  WORLD00_AREA030_DRAW_HEADER_AND_STATUS(statusWork, statusMask);
  drawEquipmentGuideColumn(16, 64);
  renderLureListColumn(164, 64);
}
