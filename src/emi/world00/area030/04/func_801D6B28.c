#include "internal.h"

/* @behavior refreshes the AREA030 HUD panel state, clamps the two tracked x
 * positions into their visible ranges, then routes the two marker helpers.
 * @source 0x801D6B28
 */
void func_801D6B28(s8 arg0) {
  volatile u8* scratch;
  s32          left_x;
  s32          right_x;
  s32          mode;

  func_8014D4E0();
  submitTpageDrawMode(0, 1);
  func_801E0DCC(0x15, 1, 0x32, 0x96);
  func_801E0DCC(0x16, 1, 0xbc, 0x96);
  func_801E0DCC(0x11, 1, 0x36, 0x9c);

  scratch = WORLD00_AREA030_SCRATCH_PTR;

  left_x = *(volatile s32*)(scratch + 0x0cu);
  if (left_x < 0x39) {
    left_x = 0x39;
  } else if (left_x >= 0x7a) {
    left_x = 0x79;
  }
  *(volatile s32*)(scratch + 0x0cu) = left_x;

  right_x = *(volatile s32*)(scratch + 0x10u);
  if (right_x < 0x41) {
    right_x = 0x41;
  } else if (right_x >= 0x95) {
    right_x = 0x94;
  }
  *(volatile s32*)(scratch + 0x10u) = right_x;

  if ((right_x < left_x) || (left_x + 0x20 < right_x)) {
    mode = 1;
  } else {
    mode = 0;
  }

  func_801D3244((s16)left_x, 0xb3, 0x20u, (s8)((scratch[6] & 7u) << 1), 1u,
                (s8)mode);
  func_801D2C34((s16)right_x, 0xb3, (s8)(arg0 != 0), 1u);
}
