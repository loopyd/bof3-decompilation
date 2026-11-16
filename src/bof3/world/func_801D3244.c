#include "bof3/world/area03004_internal.h"

/* @behavior redraws one AREA030 panel cell: selects texture-page draw mode 0 with
 * the caller mode byte, submits sprite 0x1C at the caller origin, stores the low
 * byte of arg2 in the returned cell halfword at 0x10 and adds arg3 to its byte at
 * 0x0C; when the trailing flag is set and bit 3 of the shared frame counter
 * D_80143E6C is clear it then selects draw mode 6/2, stamps a semi-transparent
 * TILE at the primitive cursor with the caller colour 0xFF, the caller origin, the
 * arg2 width and height 8, and commits it through func_8014E5A0.
 * @source 0x801D3244
 * @status exact
 * @match 100.00
 * @residual none
 */
void func_801D3244(s16 arg0, s16 arg1, u8 arg2, s8 arg3, u8 arg4, u8 arg5) {
  u8*  cell;
  TILE* tile;
  s32  shade;

  submitTpageDrawMode(0, arg4);
  cell = func_801E0DCC(0x1C, arg4, arg0, arg1);
  shade = arg2 & 0xFF;
  *(volatile u16*)(cell + 0x10u) = (u16)shade;
  cell[0x0Cu] = (u8)(cell[0x0Cu] + arg3);

  if (arg5 != 0u) {
    if ((D_80143E6C & 8) == 0) {
      submitTpageDrawMode(6, 2);
      tile = (TILE*)g_PrimCursor;
      SetTile(tile);
      SetSemiTrans(tile, 1);
      tile->x0 = arg0;
      tile->y0 = arg1;
      tile->w = (u16)shade;
      tile->h = 8;
      tile->r0 = 0xFF;
      tile->g0 = 0;
      tile->b0 = 0;
      func_8014E5A0(arg4, 0x10);
    }
  }
}
