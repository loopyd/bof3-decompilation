#include "internal.h"

/* @source 0x801DF90C
 * @behavior Initializes a TILE primitive as a full-screen semi-transparent
 *           black tile (320x240) and appends it to the primitive list.
 */
void func_801DF90C(void) {
  TILE* prim = (TILE*)g_PrimCursor;
  SetTile(prim);
  SetSemiTrans(prim, 0);
  prim->w = 0x140;
  prim->x0 = 0;
  prim->y0 = 0;
  prim->h = 0xF0;
  prim->r0 = 0;
  prim->g0 = 0;
  prim->b0 = 0;
  func_8014E5A0(1, 0x10);
}
