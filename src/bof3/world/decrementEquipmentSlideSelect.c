#include "bof3/world/area03004_internal.h"

/* @behavior Decrement work byte 9 modulo 256. A fresh zero read sets byte 0xB and either
 * advances handler byte 1 or selects handler 4 according to byte-7 bit 0. Draw
 * the moving or fixed header according to byte 0xB, the equipment guide at 16
 * - 45 * count and the rod or lure column at 164 + 45 * count.
 * @source 0x801D4378
 * @status exact
 * @match 100.00
 * @residual none
 */
void decrementEquipmentSlideSelect(void) {
  u8* countdownWork;
  u8* work;
  u8* maskCursor;
  u8 remainingSteps;
  u32 statusMask;

  countdownWork = (u8*)D_1F800044;
  countdownWork[9u] = (u8)(countdownWork[9u] - 1u);

  work = (u8*)D_1F800044;
  if (work[9u] == 0u) {
    work[11u] = 1u;
    work = (u8*)D_1F800044;
    if ((work[7u] & 1u) == 0u) {
      work[1u] = (u8)(work[1u] + 1u);
    } else {
      work[1u] = 4u;
    }
    work = (u8*)D_1F800044;
  }

  if (work[11u] == 0u) {
    remainingSteps = work[9u];
    renderFilledInsetBorder(20, (u16)((18 - (remainingSteps << 4)) & 0xFFFEu), 280, 19, 0);
    remainingSteps = ((u8*)D_1F800044)[9u];
    renderLabeledStatusCells((u16)(88 + remainingSteps * 45), 40, 0);
  } else {
    WORLD00_AREA030_DRAW_HEADER_AND_STATUS(maskCursor, statusMask);
  }

  remainingSteps = ((u8*)D_1F800044)[9u];
  drawEquipmentGuideColumn((s16)(16 - remainingSteps * 45), 64);

  work = (u8*)D_1F800044;
  if ((work[7u] & 1u) == 0u) {
    drawRodListColumn((s16)(164 + work[9u] * 45), 64);
  } else {
    renderLureListColumn(
        (s16)(164 + work[9u] * 45), 64);
  }
}
