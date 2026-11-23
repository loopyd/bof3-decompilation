#include "bof3/world/area03004_internal.h"

/* @behavior redraws one AREA030 panel sprite: selects texture-page draw mode 0 with
 * the caller mode byte, submits sprite 0x1C at the caller origin, stores the low
 * byte of width in the returned sprite halfword at 0x10 and adds uOffset to its byte at
 * 0x0C; when the trailing flag is set and bit 3 of the shared frame counter
 * D_80143E6C is clear it then selects draw mode 6/2, stamps a semi-transparent
 * TILE at the primitive cursor with the caller colour 0xFF, the caller origin, the
 * width width and height 8, and commits it through func_8014E5A0.
 * @source 0x801D3244
 * @status exact
 * @match 100.00
 * @residual none
 */
void renderOptionalBlinkingHighlight(s16 x, s16 y, u8 width, s8 uOffset, u8 drawMode, u8 highlightEnabled) {
  u8*  sprite;
  TILE* tile;
  s32  cellWidth;

  submitTpageDrawMode(0, drawMode);
  sprite = appendIndexedSprite(0x1C, drawMode, x, y);
  cellWidth = width & 0xFF;
  *(volatile u16*)(sprite + 0x10u) = (u16)cellWidth;
  sprite[0x0Cu] = (u8)(sprite[0x0Cu] + uOffset);

  if (highlightEnabled != 0u) {
    if ((D_80143E6C & 8) == 0) {
      submitTpageDrawMode(6, 2);
      tile = (TILE*)g_PrimCursor;
      SetTile(tile);
      SetSemiTrans(tile, 1);
      tile->x0 = x;
      tile->y0 = y;
      tile->w = (u16)cellWidth;
      tile->h = 8;
      tile->r0 = 0xFF;
      tile->g0 = 0;
      tile->b0 = 0;
      func_8014E5A0(drawMode, 0x10);
    }
  }
}
