#include "internal.h"

/* @behavior draws two paired frontend label groups at the requested position;
 * the first expanded pair is optional, while both groups share selection and
 * alpha controls.
 * @source 0x801D150C
 */
void func_801D150C(s16 x, s16 y, u8 selected, u8 alpha) {
  u8* primitive;
  s32 marker_x;

  if (selected != 0u) {
    marker_x = func_8017B2B4() == 1 ? 809 : (func_8017B2B4() == 2 ? 809 : 217);
    func_8017C2D8(D_8014598C, 0, 0, marker_x, 0);
    func_8014E5A0(2, 12);
    primitive = func_801D17D8(x, y, 15, 2, 1);
    func_801D18E8(primitive, alpha);
    primitive = func_801D17D8((s16)(x + 224), y, 16, 2, 1);
    func_801D18E8(primitive, alpha);
  }

  marker_x = func_8017B2B4() == 1 ? 681 : (func_8017B2B4() == 2 ? 681 : 185);
  func_8017C2D8(D_8014598C, 0, 0, marker_x, 0);
  func_8014E5A0(2, 12);
  primitive = func_801D17D8(x, y, 4, 2, selected);
  func_801D18E8(primitive, alpha);
  primitive = func_801D17D8((s16)(x + 224), y, 5, 2, selected);
  func_801D18E8(primitive, alpha);
}
