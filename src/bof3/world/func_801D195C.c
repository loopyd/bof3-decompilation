#include "bof3/world/area03004_internal.h"

/* @behavior draws the AREA030 status frame at the caller origin: the 0x801D1744
 * panel is requested at (x + 88, y + 8), the scroll offset of the two shared words
 * 0x80143FFC/0x80144000 is subtracted from y in thirty-seconds once the shared byte
 * 0x80143FC9 is set and clamped so the top never starts below y, and the 0x801D1818
 * panel is requested at (x + 89, top). A two-line pair primitive is then stamped at
 * the shared cursor from the left edge x + 89 and x + 105, the rows top + 16 and
 * y + 40, colour (72, 32, 0) and committed through func_8014E5A0. While the shared
 * byte then reads 4 one more line is drawn at (x + 105, 180 - word/32) through
 * func_801D18CC with mode 2, where the shared word 0x80144006 yields the plain 180
 * for a positive value.
 * @source 0x801D195C
 * @status partial
 * @match 95.41
 * @residual 104 of 109 current instructions match 109 original at the same
 * 436-byte size. The panels, the line pair, the colour stores and the tail rounding
 * block all agree. The first rounding block, which folds the func_8015477C result
 * out of y, keeps the shifted value in a second register and repeats the shift:
 * the original leaves both in v0 with one sra.
 */
void func_801D195C(s16 arg0, s16 arg1) {
  Area030LinePair* prim;
  s16 x;
  s16 y;
  s16 top;
  s16 left;
  s32 dy;
  s16 stamp0;
  s16 stamp1;

  x = arg0;
  y = arg1;
  func_801D1744((s16)(x + 88), (s16)(y + 8), 4);

  if (D_80143FC9 == 0u) {
    top = y;
  } else {
    dy = func_8015477C(D_80143FFC, D_80144000);
    if (dy < 0) {
      dy += 31;
    }
    dy >>= 5;
    top = y - dy;
  }
  if ((s16)top < (s16)y) {
    top = y;
  }

  left = x + 89;
  func_801D1818((s16)left, (s16)top, 4);

  prim = (Area030LinePair*)g_PrimCursor;
  func_8017A9A4(prim);
  prim->x0 = (s16)left;
  stamp0 = ((volatile Area030LinePair*)prim)->x0;
  prim->x1 = (s16)(x + 105);
  stamp1 = ((volatile Area030LinePair*)prim)->x1;
  prim->y0 = (s16)(top + 16);
  prim->y1 = (s16)(top + 16);
  prim->y2 = (s16)(y + 40);
  prim->y3 = (s16)(y + 40);
  prim->r0 = 72;
  prim->g0 = 32;
  prim->b0 = 0;
  prim->x2 = stamp0;
  prim->x3 = stamp1;
  func_8014E5A0(4, 24);

  if (D_80143FC9 == 4u) {
    dy = D_80144006;
    if (dy > 0) {
      top = 180;
    } else {
      if (dy < 0) {
        dy += 31;
      }
      dy >>= 5;
      top = 180 - dy;
    }
    func_801D18CC((s16)(x + 105), (s16)top, 2);
  }
}
