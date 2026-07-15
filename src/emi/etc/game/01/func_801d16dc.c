#include "internal.h"

/* @behavior draws a paired frontend label group at the requested position,
 * applying the supplied selection and alpha controls to both primitives.
 * @source 0x801d16dc FUN_801d16dc
 */
void func_801d16dc(s16 x, s16 y, u8 selected, u8 alpha) {
  u8* primitive;
  s32 marker_x;

  marker_x = func_8017b2b4() == 1 ? 683 : (func_8017b2b4() == 2 ? 683 : 187);
  func_8017c2d8(D_8014598C, 0, 0, marker_x, 0);
  func_8014e5a0(2, 12);
  primitive = func_801d17d8(x, y, 2, 2, selected);
  func_801d18e8(primitive, alpha);
  primitive = func_801d17d8((s16)(x + 240), (s16)(y + 112), 3, 2, selected);
  func_801d18e8(primitive, alpha);
}
