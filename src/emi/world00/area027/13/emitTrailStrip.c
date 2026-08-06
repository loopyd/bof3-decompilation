#include "internal.h"

/* @behavior emits a 31-segment Gouraud line strip across the projected trail,
 * fading the leading endpoint from bright red toward black.
 * @source 0x801F2F0C
 */
void emitTrailStrip(const void* arg0) {
  u8*      points;
  LINE_G2* line;
  u8       i;
  u32      j;
  u32      off;
  u16      red;

  points = (u8*)arg0;

  SetDrawMode((DR_MODE*)WORLD00_AREA027_PRIMITIVE_PTR, 0, 0,
              GetTPage(0, 1, 0x380, 0x100), 0);
  func_8014E5A0(1u, 0x0cu);

  i = 0u;
  do {
    line = (LINE_G2*)WORLD00_AREA027_PRIMITIVE_PTR;
    func_8017AA94(line);
    SetSemiTrans(line, 1);

    j = i + 1u;

    off = (u32)i * 4u;
    line->x0 = *(s16*)(points + off + 0x18u);
    line->y0 = *(s16*)(points + off + 0x1au);
    off = j * 4u;
    line->x1 = *(s16*)(points + off + 0x18u);
    line->y1 = *(s16*)(points + off + 0x1au);

    red = (0x20u - (u16)i) * 8u;
    if (red > 0xffu) {
      red = 0xffu;
    }
    setRGB0(line, red, 0u, 0u);
    red = (0x1fu - (u16)i) * 8u;
    if (red > 0xffu) {
      red = 0xffu;
    }
    setRGB1(line, red, 0u, 0u);
    func_8014E5A0(1u, 0x14u);

    i += 1u;
  } while (i < 0x1fu);
}
