#include "internal.h"

/* @behavior draws a paired frontend label group at the requested position,
 * applying the supplied selection and alpha controls to both primitives.
 * @source 0x801D16DC
 */
void func_801D16DC(s16 x, s16 y, u8 selected, u8 alpha) {
  u8* primitive;
  s32 marker_x;

  marker_x = func_8017B2B4() == 1 ? 683 : (func_8017B2B4() == 2 ? 683 : 187);
  func_8017C2D8(D_8014598C, 0, 0, marker_x, 0);
  func_8014E5A0(2, 12);
  primitive = func_801D17D8(x, y, 2, 2, selected);
  func_801D18E8(primitive, alpha);
  primitive = func_801D17D8((s16)(x + 240), (s16)(y + 112), 3, 2, selected);
  func_801D18E8(primitive, alpha);
}
