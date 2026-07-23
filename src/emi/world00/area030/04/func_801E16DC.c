#include "internal.h"

/* @behavior configures the dim semi-transparent AREA030 tile and appends it
 * after selecting graphics mode 4.
 * @source 0x801E16DC
 */
void func_801E16DC(void) {
  TILE* primitive;

  func_801E0C80(4, 2);
  primitive = (TILE*)D_8014598C;
  SetTile(primitive);
  primitive->b0 = 0x20;
  primitive->g0 = 0x20;
  primitive->r0 = 0x20;
  primitive->w = 0x140;
  primitive->x0 = 0;
  primitive->y0 = 0;
  primitive->h = 0xf0;
  SetSemiTrans(primitive, 1);
  func_8014E5A0(2, 0x10);
}
