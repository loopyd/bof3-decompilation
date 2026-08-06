#include "internal.h"

/* @behavior advances the local scratch state when the loader poll succeeds, then
 * emits the two fixed world markers through `func_801F3480`.
 * @source 0x801F31CC
 */
void world00_area027_marker_pair_emit(void) {
  s16 point[3];

  if (func_8015B5D4((u32)D_80144E98, 0x17) != 0) {
    WORLD00_AREA027_SCRATCH_PTR[2] += 1u;
    *(u32*)(WORLD00_AREA027_SCRATCH_PTR + 0x0c) = 0u;
  }

  point[0] = (s16)0xe340u;
  point[1] = (s16)0xe3c0u;
  point[2] = 0;
  func_801F3480(point, 0, 0x13500126u);
  func_80155A08(0x468000, 0x478000, 0, 0x28);

  point[0] = (s16)0xe340u;
  point[1] = (s16)0xe4c0u;
  point[2] = 0;
  func_801F3480(point, 0x800, 0x13510125u);
  func_80155A08(0x468000, 0x498000, -1, 0x28);
}
