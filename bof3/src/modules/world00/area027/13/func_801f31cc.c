#include "internal.h"

/* does: advances the local scratch state when the loader poll succeeds, then
 * emits the two fixed world markers through `func_801f3480`.
 * @source: 0x801f31cc FUN_801f31cc
 */
void func_801f31cc(void) {
  s16 point[3];

  if (func_8015b5d4(0x80144e98u, 0x17) != 0) {
    WORLD00_AREA027_SCRATCH_PTR[2] += 1u;
    *(u32*)(WORLD00_AREA027_SCRATCH_PTR + 0x0c) = 0u;
  }

  point[0] = (s16)0xe340u;
  point[1] = (s16)0xe3c0u;
  point[2] = 0;
  func_801f3480(point, 0, 0x13500126u);
  func_80155a08(0x468000, 0x478000, 0, 0x28);

  point[0] = (s16)0xe340u;
  point[1] = (s16)0xe4c0u;
  point[2] = 0;
  func_801f3480(point, 0x800, 0x13510125u);
  func_80155a08(0x468000, 0x498000, -1, 0x28);
}
