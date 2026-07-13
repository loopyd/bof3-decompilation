#include "internal.h"

/* @behavior draws two paired frontend label groups at the requested position;
 * the first expanded pair is optional, while both groups share selection and
 * alpha controls.
 * @source 0x801d150c FUN_801d150c
 */
void func_801d150c(s16 x, s16 y, u8 selected, u8 alpha) {
  u8* primitive;
  s32 marker_x;

  if (selected != 0u) {
    marker_x = func_8017b2b4() == 1 ? 809 : (func_8017b2b4() == 2 ? 809 : 217);
    func_8017c2d8(DAT_8014598c, 0, 0, marker_x, 0);
    func_8014e5a0(2, 12);
    primitive = func_801d17d8(x, y, 15, 2, 1);
    func_801d18e8(primitive, alpha);
    primitive = func_801d17d8((s16)(x + 224), y, 16, 2, 1);
    func_801d18e8(primitive, alpha);
  }

  marker_x = func_8017b2b4() == 1 ? 681 : (func_8017b2b4() == 2 ? 681 : 185);
  func_8017c2d8(DAT_8014598c, 0, 0, marker_x, 0);
  func_8014e5a0(2, 12);
  primitive = func_801d17d8(x, y, 4, 2, selected);
  func_801d18e8(primitive, alpha);
  primitive = func_801d17d8((s16)(x + 224), y, 5, 2, selected);
  func_801d18e8(primitive, alpha);
}
