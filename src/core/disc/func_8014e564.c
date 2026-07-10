#include "internal.h"

/* does: clears one rectangular display region.
 * @source: 0x8014e564 FUN_8014e564
 */
void func_8014e564(s16 x, s16 y, s16 width, s16 height) {
  RECT rect;

  rect.x = x;
  rect.y = y;
  rect.w = width;
  rect.h = height;
  ClearImage(&rect, 0, 0, 0);
}
