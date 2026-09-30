#include "bof3/ui/sisyou00_internal.h"

/* @source 0x801D1CC0
 * @behavior builds the three-byte colour-channel mask of the emitted flat-line
 *           rectangle from the low three bits of the mask argument: each set bit
 *           selects the flat colour (0x3F | ((D_80143E6C & 8) ? (D_80143E6C & 6)
 *           : (~D_80143E6C & 6)) << 5 when the colour-enable argument is set,
 *           0xFF otherwise) and each clear bit selects black; it then draws an
 *           open three-point flat line from (x+4, y) to (x, y+4) to (x, y+h-1)
 *           with SetLineF3 and the closed four-corner border (x+4, y),
 *           (x+w-1, y), (x+w-1, y+h-1), (x, y+h-1) with SetLineF4, queueing each
 *           0x18/0x1C-byte primitive through func_8014E5A0(1, size).
 * @status exact
 * @match 100.00
 * @residual none
 */
void func_801D1CC0(u16 x, u16 y, u16 w, u16 h, u8 colorEnable, u8 mask) {
  u8 channels[3];
  u32 bit;
  u32 maskBits = mask;
  u16 rightEdge;
  u8 i;
  u32 color;
  s32 flags;
  LINE_F3 *line3;
  LINE_F4 *line4;

  if (colorEnable != 0) {
    flags = D_80143E6C;
    if ((flags & 8) != 0) {
      color = ((flags & 6) << 5) | 0x3F;
    } else {
      color = ((~flags & 6) << 5) | 0x3F;
    }
  } else {
    color = 0xFF;
  }

  bit = 1;
  i = 0;
  do {
    if (bit & maskBits) {
      channels[i] = color;
    } else {
      channels[i] = 0;
    }
    i++;
    bit <<= 1;
  } while (i < 3);

  line3 = (LINE_F3 *)g_PrimCursor;
  SetLineF3(line3);
  line3->x0 = x + 4;
  line3->y0 = y;
  line3->x1 = x;
  line3->y1 = y + 4;
  line3->x2 = x;
  line3->y2 = y + h - 1;
  line3->r0 = channels[2];
  line3->g0 = channels[1];
  line3->b0 = channels[0];
  func_8014E5A0(1, 0x18);

  line4 = (LINE_F4 *)g_PrimCursor;
  SetLineF4(line4);
  rightEdge = x + w - 1;
  line4->x0 = x + 4;
  line4->y0 = y;
  line4->x3 = x;
  line4->y3 = y + h - 1;
  line4->y1 = y;
  line4->y2 = y + h - 1;
  line4->x2 = rightEdge;
  line4->x1 = rightEdge;
  line4->r0 = channels[2];
  line4->g0 = channels[1];
  line4->b0 = channels[0];
  func_8014E5A0(1, 0x1C);
}
