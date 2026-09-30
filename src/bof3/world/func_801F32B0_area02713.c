#include "bof3/world/area02713_internal.h"

/**
 * @source 0x801F32B0
 * @behavior Advances the local scratch cursor word at offset 0x0c by 0x20,
 * raises scratch byte 2 once that cursor has reached 0x400, then emits the two
 * fixed world markers through `emitMarkerQuad` with the cursor halfword and
 * with 0x800 minus the cursor, each followed by a fixed line draw.
 * @status exact
 * @match 100.00
 * @residual none
 */
void func_801F32B0(void) {
  s16 point[3];

  if (*(s32*)(WORLD00_AREA027_SCRATCH_PTR + 0x0c) >= 0x400) {
    WORLD00_AREA027_SCRATCH_PTR[2] += 1u;
  }

  *(s32*)(WORLD00_AREA027_SCRATCH_PTR + 0x0c) += 0x20;
  point[0] = (s16)0xe340u;
  point[1] = (s16)0xe3c0u;
  point[2] = 0;
  emitMarkerQuad(point, *(s16*)(WORLD00_AREA027_SCRATCH_PTR + 0x0c),
                 0x13500126u);
  func_80155A08(0x468000, 0x478000, -1, 0x28);

  point[0] = (s16)0xe340u;
  point[1] = (s16)0xe4c0u;
  point[2] = 0;
  emitMarkerQuad(point, (s16)(0x800 - *(u32*)(WORLD00_AREA027_SCRATCH_PTR + 0x0c)),
                 0x13510125u);
  func_80155A08(0x468000, 0x498000, -1, 0x28);
}
