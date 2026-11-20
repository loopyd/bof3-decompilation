#include "bof3/world/area03004_internal.h"

/* @behavior Draw header sprites 0x0D, 0x0E and 0x0F. Use zero when D_80143FC9 is clear;
 * otherwise interpret the low byte of 62 - D_80144002 as signed and replace a
 * negative result with zero. Format that signed byte at x + 240, y + 26. Text
 * mode 2 requires a value of at least 50, shared phase 4 and frame bit 2. Draw
 * the marker frame last.
 * @source 0x801D159C
 * @status exact
 * @match 100.00
 * @residual none
 */
void drawStatusHeaderCounterAndMarkerFrameWorld00Area03004(s16 x, s16 y) {
  u8* textBuffer;
  s32 counterValue;

  submitTpageDrawMode(5, 3);
  appendIndexedSpriteWorld00Area03004(0x0D, 3, (s16)(x + 24), (s16)(y + 8));
  submitTpageDrawMode(0, 3);
  appendIndexedSpriteWorld00Area03004(0x0E, 3, x, y);
  appendIndexedSpriteWorld00Area03004(0x0F, 3, (s16)(x + 256), y);

  if (D_80143FC9 == 0u) {
    counterValue = 0;
  } else {
    counterValue = 62 - D_80144002;
  }
  if ((s8)counterValue < 0) {
    counterValue = 0;
  }
  textBuffer = D_80145AD4;
  func_8017E3F4((char*)textBuffer, (char*)D_801D0C04, (s8)counterValue);

  if (((s8)counterValue >= 50) && (D_80145E92 == 4u) && ((D_80143E6C & 4) != 0)) {
    func_8014FF0C((s16)(x + 240), (s16)(y + 26), 2, D_80145AD4);
  } else {
    func_8014FF0C((s16)(x + 240), (s16)(y + 26), 0, D_80145AD4);
  }
  drawStatusMarkerFrameWorld00Area03004(x, y);
}
