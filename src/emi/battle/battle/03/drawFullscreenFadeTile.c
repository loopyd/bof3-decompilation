#include "internal.h"

/* @source 0x801D9900
 * @behavior emits a semi-transparent full-screen tile primitive
 */
void drawFullscreenFadeTile(void) {
  TILE* tile;

  SetDrawMode((DR_MODE*)D_8014598C, 0, 0, GetTPage(0, 2, 0x3c0, 0), 0);
  func_8014E5A0(1, 0x0c);
  tile = (TILE*)D_8014598C;
  SetTile(tile);
  tile->w = 0x3c0;
  tile->h = 0xf0;
  tile->x0 = 0;
  tile->y0 = 0;
  tile->r0 = 0x28;
  tile->g0 = 0x28;
  tile->b0 = 0x28;
  SetSemiTrans(tile, 1);
  func_8014E5A0(1, 0x10);
}
