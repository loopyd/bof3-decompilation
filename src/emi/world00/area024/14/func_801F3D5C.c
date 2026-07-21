#include "internal.h"

/* @behavior draws the 16-entry spin table and advances each entry's size/fade
 * halfwords using the scratch byte at `0x1f80004d`.
 * @source 0x801F3D5C
 */
void func_801F3D5C(void) {
  u8                      scratch[0x20];
  World00Area024SpinWork* work;
  u8                      i;

  SetDrawMode((DR_MODE*)WORLD00_AREA024_PRIMITIVE_PTR, 0, 0,
              GetTPage(0, 1, 0x380, 0x100), 0);
  func_8014E5A0(1u, 0x0cu);
  func_801AFE18(scratch);

  work = (World00Area024SpinWork*)WORLD00_AREA024_SPIN_WORK_BASE;
  i = 0u;

  do {
    func_801F3944(work);
    work->field_24 = (s16)(work->field_24 + 0x10);

    if (WORLD00_AREA024_SCRATCH_BYTE_09 < 4u) {
      work->field_2a = (s16)(work->field_2a + 0x20);
    } else {
      work->field_2a = (s16)(work->field_2a - 2);
    }

    work = (World00Area024SpinWork*)((u8*)work + 0x2cu);
    i += 1u;
  } while (i < 0x10u);
}
