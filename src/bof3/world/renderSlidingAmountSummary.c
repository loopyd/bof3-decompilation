#include "bof3/world/area03004_internal.h"

typedef struct PackedCoords {
  u8 value[6];
} PackedCoords;

/* @source 0x801E1004
 * @behavior Draw three decorations using the low byte of slideCount. Copy 12 bytes for
 * entry 0x16 or 0xFF, otherwise copy at most 12 bytes from the selected
 * 18-byte label row, then terminate and draw the sliding label. Except for
 * entry 0xFF, format the low byte of amount and the low halfword of the scaled
 * slot value, replace the first three spaces with 0xFF and draw the amount
 * summary.
 * @status exact
 * @match 100.00
 * @residual none
 */
void renderSlidingAmountSummary(s32 entryId, s32 amount, s32 slideCount) {
  PackedCoords decorationCoords;
  s32 index;
  u8* textCursor;
  u8* labelRow;

  decorationCoords = *(PackedCoords*)D_801D0C30;
  submitTpageDrawMode(0, 1);
  for (index = 0; index < 3; index++) {
    s32 x;
    s32 y;
    if (index == 0) {
      x = decorationCoords.value[0];
      y = decorationCoords.value[1] - ((slideCount & 0xff) * 20);
    } else if (index == 1) {
      x = decorationCoords.value[2] - ((slideCount & 0xff) * 25);
      y = decorationCoords.value[3];
    } else {
      s32 coordinateOffset = index << 1;
      const u8* coordinatePair = (const u8*)&decorationCoords + coordinateOffset;
      x = coordinatePair[0] + ((slideCount & 0xff) * 25);
      y = coordinatePair[1];
    }
    appendIndexedSpriteWorld00Area03004((index + 3) & 0xff, 1, (s16)x, (s16)y);
  }

  textCursor = D_80145AD4;
  if ((entryId & 0xff) == 0x16) {
    for (index = 0; index < 12; index++) *textCursor++ = D_801E28C4[index];
  } else if ((entryId & 0xff) == 0xff) {
    for (index = 0; index < 12; index++) *textCursor++ = D_801E28D0[index];
  } else {
    index = 0;
    labelRow = D_801C8964 + (((entryId & 0xff) + 0x38) * 18);
    while (index < 12) {
      *textCursor = *labelRow;
      if (*labelRow == 0) {
        index++;
        break;
      }
      index++;
      textCursor++;
      labelRow++;
    }
  }
  *textCursor = 0;
  drawEncodedTextAt(0x70, (s16)(0x45 - ((u8)slideCount * 20)), 0, 0xf, (u32)D_80145AD4);

  if ((entryId & 0xff) != 0xff) {
    sprintf((char*)D_80145AD4, (const char*)D_801D0C38, amount & 0xff);
    for (index = 0; index < 3; index++) {
      if (D_80145AD4[index] == 0x20) D_80145AD4[index] = 0xff;
    }
    drawEncodedTextAt((s16)(0x2f - ((u8)slideCount * 25)), 0x73, 0, 3, (u32)D_80145AD4);
    sprintf((char*)D_80145AD4, (const char*)D_801D0C38,
            calculateScaledValueAmount(entryId & 0xff, amount & 0xff) & 0xffff);
    for (index = 0; index < 3; index++) {
      if (D_80145AD4[index] == 0x20) D_80145AD4[index] = 0xff;
    }
    drawEncodedTextAt((s16)(((u8)slideCount * 25) + 0xef), 0x73, 0, 3,
                  (u32)D_80145AD4);
  }
}
