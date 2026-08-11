#include "bof3/battle/battle03_internal.h"

/* @source 0x801D9C80
 * @behavior emits a colored tile using a palette row selected by battle state
 * @status partial
 * @match 53.16
 * @residual missing source shape leaves a 60-byte size deficit
 */
void func_801D9C80(s16 arg0, s16 arg1, s32 arg2, s32 arg3) {
  TILE* tile;
  u16 color;
  s32 index;

  tile = (TILE*)g_PrimCursor;
  SetTile(tile);
  tile->x0 = arg0;
  tile->y0 = arg1;
  tile->w = D_801EAE50[(u8)arg2][0];
  tile->h = D_801EAE50[(u8)arg2][1];

  index = ((D_80144952 * 2) + (u8)arg3) * 16;
  color = D_80033A08[index];
  tile->r0 = (color & 0x1f) << 3;
  color = D_80033A08[index];
  tile->g0 = (color >> 2) & 0xf8;
  color = D_80033A08[index];
  tile->b0 = (color >> 7) & 0xf8;
  SetSemiTrans(tile, (u8)arg3);
  func_8014E5A0(1, 0x10);
}
