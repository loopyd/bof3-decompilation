#include "bof3/bof3.h"

extern u8 D_80145E98;   /* @source 0x80145E98 @kind data */
extern u8 D_80181B00[]; /* @source 0x80181B00 @kind table */
extern u8 D_80181B01[]; /* @source 0x80181B01 @kind table */

void func_801A4BC0(s16 x, s16 y, u32 size);

/* @source 0x801F372C
 * @behavior Reads the shared route index from the shared mode-record byte
 *           D_80145E98 masked with 7 and takes that entry (index times two) of
 *           the shared direction-step table D_80181B00 / D_80181B01 as two
 *           signed byte steps. Each of the two packed position arguments keeps
 *           its low halfword as the fraction while the step is subtracted from
 *           its cell halfword; when the shifted first cell lies in 0x94..0x97
 *           and the shifted second cell lies in 0x20..0x23, the shared UI
 *           element at that shifted cell pair is queued through func_801A4BC0
 *           with size 0x50. Returns nothing.
 * @status exact
 * @match 100.00
 * @residual none
 */
void queueSharedUiElementSize50AtShiftedCellInsideMarkerBlockWorld01Area04013_801F372C(
    s32 pos0, s32 pos1) {
  s16 cell0;
  s16 cell1;
  s32 off;
  s32 x;
  s32 y;

  cell0 = (s16)(pos0 >> 16);
  pos0 = pos0 & 0xFFFF;
  cell1 = (s16)(pos1 >> 16);
  pos1 = pos1 & 0xFFFF;
  off = (D_80145E98 & 7) * 2;
  cell0 -= (s8)D_80181B00[off];
  pos0 = pos0 | (cell0 << 16);
  cell1 -= (s8)D_80181B01[off];
  pos1 = pos1 | (cell1 << 16);
  x = pos0 >> 16;
  if ((u32)(x - 0x94) < 4) {
    y = pos1 >> 16;
    if ((u32)(y - 0x20) < 4) {
      func_801A4BC0(x, y, 0x50);
    }
  }
}
