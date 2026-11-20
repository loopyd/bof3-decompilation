#include "bof3/world/area03004_internal.h"

/* @behavior Draw the fixed quad at originX + 88, originY + 8. When D_80143FC9 is set,
 * subtract the signed-halfword helper result divided by 32 towards zero from
 * originY, then clamp the signed-halfword top to at least originY. Draw the
 * small panel and line pair. At mode 4, draw the 8x8 marker using the original
 * signed-halfword counter adjustment and volatile coordinate readbacks.
 * @source 0x801D195C
 * @status exact
 * @match 100.00
 * @residual none
 */
void drawStatusMarkerFrameWorld00Area03004(s16 originX, s16 originY) {
  Area030LinePair* linePair;
  s16 x;
  s16 y;
  s16 top;
  s16 leftX;
  s32 markerOffset;
  s16 leftStamp;
  s16 rightStamp;

  x = originX;
  y = originY;
  drawBlueGradientQuad16x32World00Area03004((s16)(x + 88), (s16)(y + 8), 4);

  if (D_80143FC9 == 0u) {
    top = y;
  } else {
    s32 scrollOffset;
    scrollOffset = func_8015477C(D_80143FFC, D_80144000);
    if (scrollOffset < 0) {
      scrollOffset += 31;
    }
    scrollOffset >>= 5;
    top = y - scrollOffset;
  }
  if ((s16)top < (s16)y) {
    top = y;
  }

  leftX = x + 89;
  drawCounterIndexedSprite16x16World00Area03004((s16)leftX, (s16)top, 4);

  linePair = (Area030LinePair*)g_PrimCursor;
  func_8017A9A4(linePair);
  linePair->x0 = (s16)leftX;
  leftStamp = ((volatile Area030LinePair*)linePair)->x0;
  linePair->x1 = (s16)(x + 105);
  rightStamp = ((volatile Area030LinePair*)linePair)->x1;
  linePair->y0 = (s16)(top + 16);
  linePair->y1 = (s16)(top + 16);
  linePair->y2 = (s16)(y + 40);
  linePair->y3 = (s16)(y + 40);
  linePair->r0 = 72;
  linePair->g0 = 32;
  linePair->b0 = 0;
  linePair->x2 = leftStamp;
  linePair->x3 = rightStamp;
  func_8014E5A0(4, 24);

  if (D_80143FC9 == 4u) {
    markerOffset = D_80144006;
    if (markerOffset > 0) {
      top = 180;
    } else {
      if (markerOffset < 0) {
        markerOffset += 31;
      }
      markerOffset >>= 5;
      top = 180 - markerOffset;
    }
    drawFixedCellSprite8x8World00Area03004((s16)(x + 105), (s16)top, 2);
  }
}
