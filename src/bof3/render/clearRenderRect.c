#include "bof3/core/slus_internal.h"

/* @behavior clears one rectangular display region.
 * @source 0x8014E564
 * @status exact
 * @match 100.00
 * @residual none; live audit is instruction- and byte-exact.
 */
void clearRenderRect(s16 x, s16 y, s16 width, s16 height) {
  RECT rect;

  rect.x = x;
  rect.y = y;
  rect.w = width;
  rect.h = height;
  ClearImage(&rect, 0, 0, 0);
}
