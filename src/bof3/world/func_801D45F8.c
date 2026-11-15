#include "bof3/world/area03004_internal.h"

/* @behavior issues the fixed five-argument panel request, then draws the AREA030
 * panel variants: the third drawer argument is zero while the shared byte at
 * 0x80145E93 is below three, and otherwise a mask built from bit 0 shifted by the
 * scratch record byte at +6.
 * @source 0x801D45F8
 * @status exact
 * @match 100.00
 * @residual none
 */
void func_801D45F8(void) {
  u8* cursor;
  u32 mask;

  WORLD00_AREA030_DRAW_HEADER_AND_STATUS(cursor, mask);
  drawEquipmentGuideColumn(16, 64);
  drawLureListColumnWorld00Area03004_801D8364(164, 64);
}
